#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FILE_NAME "./TestBlockDevice"

/* modes */
#define IALLOC	0100000
#define IFMT	060000
#define IFDIR	040000
#define IFCHR	020000
#define IFBLK	060000
#define ILARG	010000
#define ISUID	04000
#define ISGID	02000
#define ISVTX	01000
#define IREAD	0400
#define IWRITE	0200
#define IEXEC	0100

#define IREAD_USR	040
#define IWRITE_USR	020
#define IEXEC_USR	010
#define IREAD_STA	04
#define IWRITE_STA	02
#define IEXEC_STA	01


/* 512byte */
struct filsys{
	int s_isize; // type of int is 2byte
	int s_fsize;
	int s_nfree;
	int s_free[100];
	int s_ninode;
	int s_inode[100];
	char s_flock;
	char s_ilock;
	char s_fmod;
	char s_ronly;
	int s_time[2];
	int pad[50];
}*super;


struct inode{

	/* 32byte */
	int i_mode;
	char i_nlink;
	char i_uid;
	char i_gid;
	char i_size0;
	char *i_size1; // 2byte
	int i_addr[8];
	int i_atime[2];
	int i_mtime[2];

	unsigned char *strage[8];
}*inodes;


struct dir_file{
	int i_num;
	char name[128];
	int df_num;
	struct dir_file *df_child[256];
}*cur;


unsigned char *boot;

static void print_bin(int value){
	unsigned int bit = (1 << (sizeof(int) * 8 - 1));
		for ( ; bit != 0 ; bit >>= 1 ){
			if ( value & bit )
				putchar('1');
			else
				putchar('0');
		}
	printf("\n");
}

static int get_16bit_big(unsigned char *mem)
{
	int ret=0,i;

	for(i=0;i<2;i++)
		ret = ret*(16*16) + (int)mem[i];

	return ret;
}

static int get_16bit_lit(unsigned char *mem)
{
	int ret=0,lit[2],i;

	for(i=0;i<2;i++)
		lit[i] = (int)mem[i];

	//for(i=0;i<2;i++)
	for(i=1;i>=0;i--)
		ret = ret*(16*16)+lit[i];

	return ret;
}

static int inode_num=0;
static void get_inode_block (unsigned char *block)
{
	struct inode *inode_block;
	int i,j;

	for (j=0;j<16;j++){
		inode_block = malloc(sizeof(struct inode));
		
		inode_block->i_mode = get_16bit_lit(block);
		block+=2;
		inode_block->i_nlink = *block;
		block++;
		inode_block->i_uid = *block;
		block++;
		inode_block->i_gid = *block;
		block++;
		inode_block->i_size0 = *block;
		block++;
		inode_block->i_size1 = get_16bit_lit(block);
		block+=2;
		for(i=0;i<8;i++){
			inode_block->i_addr[i] = get_16bit_lit(block);
			block+=2;
		}
		for(i=0;i<2;i++){
			inode_block->i_atime[i] = get_16bit_lit(block);
			block+=2;
		}
		for(i=0;i<2;i++){
			inode_block->i_mtime[i] = get_16bit_lit(block);
			block+=2;
		}
		inodes[inode_num] = *inode_block;
		inode_num++;
	}
}

static void get_super (unsigned char *block)
{
	int i;
	super = malloc(sizeof(struct filsys));

	super->s_isize = get_16bit_lit(block);
	block+=2;
	super->s_fsize = get_16bit_lit(block);
	block+=2;
	super->s_nfree = get_16bit_lit(block);
	block+=2;
	for(i=0;i<100;i++){
		super->s_free[i] = get_16bit_lit(block);
		block+=2;
	}
	super->s_ninode= get_16bit_lit(block);
	block+=2;
	for(i=0;i<100;i++){
		super->s_inode[i] = get_16bit_lit(block);
		block+=2;
	}
	super->s_flock = *block;
	block++;
	super->s_ilock = *block;
	block++;
	super->s_fmod = *block;
	block++;
	super->s_ronly = *block;
	block++;
	for(i=0;i<2;i++){
		super->s_time[i] = get_16bit_lit(block);
		block+=2;
	}
	for(i=0;i<50;i++){
		super->pad[i] = get_16bit_lit(block);
		block+=2;
	}
}

static void exit_filer (void)
{
	free(boot);
	free(super);
	free(inodes);
}

static void check_drxw(int i_num, int flag, char c)
{
	if(inodes[i_num].i_mode & flag)
		printf("%c",c);
	else
		printf("-");
}

static void init_child (void)
{
	int i;

	if(!cur)
		return;

	for(i=0;i<256;i++){
		if(cur->df_child[i])
			cur->df_child[i]=NULL;
	}
}

static void cd_cur (int i_num)
{
	int i,j,k,l;
	struct dir_file *child;
	free(cur);
	cur = malloc(sizeof(struct dir_file));
	init_child();
	for(j=0;j<8;j++){
		if(inodes[i_num].i_addr[j]){
			for (k=0;k<32;k++){
				if(*(inodes[i_num].strage[j]+k*16)){
					child = malloc(sizeof(struct dir_file));
					child->i_num = (get_16bit_lit(inodes[i_num].strage[j]+k*16))-1;
					for(l=2,i=0;l<16;l++,i++)
						child->name[i] = inodes[i_num].strage[j][k*16+l];
					child->name[i] = 0;
					cur->df_child[j*32+k] = child;
					cur->df_num++;
				}
			}
		}
	}
}

static void cat_data (int i_num)
{
	int i,j;
	for(j=0;j<8;j++){
		if(inodes[i_num].i_addr[j]){
			for(i=0;i<512;i++)
				printf("%c",inodes[i_num].strage[j][i]);
		}
	}
	printf("\n");
}

static void operation_dev (char *file_name)
{
	FILE *fp;
	unsigned char *block_super = malloc(512);
	unsigned char *block_inode = malloc(512);
	int i,j,k,l,df_num;
	char com[256],*com1,*com2;
	boot = malloc(512);

	if((fp = fopen(file_name, "rb")) == NULL ) {
		printf("file open error\n");
		exit(EXIT_FAILURE);
	}

	fread(boot, sizeof(unsigned char), 512, fp);
	fread(block_super, sizeof(unsigned char), 512, fp);
	get_super(block_super);

	inodes = malloc((sizeof(struct inode *))*super->s_isize);
	for(i=0;i<=super->s_isize;i++){
		fread(block_inode, sizeof(unsigned char), 512, fp);
		get_inode_block(block_inode);
	}

	for(i=0;i<inode_num;i++){
		for(j=0;j<8;j++){
			if(inodes[i].i_addr[j]){
				fseek(fp,(512*inodes[i].i_addr[j]),SEEK_SET);
				inodes[i].strage[j]=malloc(512);
				fread(inodes[i].strage[j], sizeof(unsigned char), 512, fp);
			}
		}
	}

	/* init */
	cd_cur(0);

	/* operation */
	while(1){
		printf(">>");
		scanf("%[^\r\n]",com);

		i=0;
		com1=&com[i];
		while(com[i] && com[i]!=' ')
			i++;
		if(com[i]){
			com[i]=0;
			i++;
			com2=&com[i];
		}

		if(com1 && !strcmp(com1,"ls")){
			i=0;
			while(i < cur->df_num && cur->df_child[i]){
				if(com2 && !strcmp(com2,"-l")){
					check_drxw(cur->df_child[i]->i_num,IFDIR,'d');
					check_drxw(cur->df_child[i]->i_num,IREAD,'r');
					check_drxw(cur->df_child[i]->i_num,IWRITE,'w');
					check_drxw(cur->df_child[i]->i_num,IEXEC,'x');
					check_drxw(cur->df_child[i]->i_num,IREAD_USR,'r');
					check_drxw(cur->df_child[i]->i_num,IWRITE_USR,'w');
					check_drxw(cur->df_child[i]->i_num,IEXEC_USR,'x');
					check_drxw(cur->df_child[i]->i_num,IREAD_STA,'r');
					check_drxw(cur->df_child[i]->i_num,IWRITE_STA,'w');
					check_drxw(cur->df_child[i]->i_num,IEXEC_STA,'x');
					printf("  ");
					if (inodes[cur->df_child[i]->i_num].i_size0)
						printf("%d",(int)inodes[cur->df_child[i]->i_num].i_size0);
					printf("%d",(int)inodes[cur->df_child[i]->i_num].i_size1);
				}
				printf(" %s",cur->df_child[i]->name);
				printf("\n");
				i++;
			}

		}else if(com1 && !strcmp(com1,"cd")){
			df_num=cur->df_num;
			for(i=0;i<df_num;i++){
				if(com2 && !strcmp(com2,cur->df_child[i]->name)){
					if(inodes[cur->df_child[i]->i_num].i_mode & IFDIR){
						//printf("[%d]\n",cur->df_child[i]->i_num);
						cd_cur(cur->df_child[i]->i_num);
						break;
					}else{
						printf("-v6sh: cd: %s : Not a directory\n",cur->df_child[i]->name);
						break;
					}
				}
			}
			if(i>=df_num)
				printf("-v6sh: cd: %s : No such file or directory\n",com2);

		}else if(com1 && !strcmp(com1,"cat")){
			df_num=cur->df_num;
			for(i=0;i<df_num;i++){
				if(com2 && !strcmp(com2,cur->df_child[i]->name)){
					if(!(inodes[cur->df_child[i]->i_num].i_mode & IFDIR)){
						cat_data(cur->df_child[i]->i_num);
						break;
					}else{
						printf("-v6sh: cat: %s : Not a file\n",cur->df_child[i]->name);
						break;
					}
				}
			}
			if(i>=df_num)
				printf("-v6sh: cat: %s : No such file or directory\n",com2);

		} else if(com1)
			printf("-v6sh: %s : command not found\n",com1);

		com1=NULL;
		com2=NULL;
		fflush(stdin);
		fseek(stdin, 0L, SEEK_SET);
	}

	fclose(fp);
	free(block_super);
	free(block_inode);
	exit_filer();
}

int main()
{
	operation_dev(FILE_NAME);
	exit(EXIT_SUCCESS);
} 
