#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <math.h>
#include "ext2.h"
#include <sys/types.h>
#include <sys/stat.h>

#define TRUE 1
#define FALSE 0

#define BASE_OFFSET 1024
#define EXT2_BLOCK_SIZE 1024
#define IMAGE "image.img"

typedef unsigned char bmap;
#define __NBITS (8 * (int) sizeof (bmap))
#define __BMELT(d) ((d) / __NBITS)
#define __BMMASK(d) ((bmap) 1 << ((d) % __NBITS))
#define BM_SET(d, set) ((set[__BMELT (d)] |= __BMMASK (d)))
#define BM_CLR(d, set) ((set[__BMELT (d)] &= ~__BMMASK (d)))
#define BM_ISSET(d, set) ((set[__BMELT (d)] & __BMMASK (d)) != 0)

unsigned int block_size = 0;
#define BLOCK_OFFSET(block) (BASE_OFFSET + (block-1)*block_size)

using namespace std;

bool isPower(int y){
	int res1 = log(y)/log(3);
	double res2 = log(y)/log(3);

	int res3 = log(y)/log(5);
	double res4 = log(y)/log(5);

	int res5 = log(y)/log(7);
	double res6 = log(y)/log(7);

	return(res1==res2 || res3==res4 || res5==res6);
}

void writeBlocktoSuper(int fd,struct ext2_super_block super)
{
  int n = ceil(super.s_blocks_count/super.s_blocks_per_group);
  int group_size=(3*n+3)*EXT2_BLOCK_SIZE;
  int group_size1=(2*n+2)*EXT2_BLOCK_SIZE;
  int gl=0;int gs=0;
  for(int i=0;i<n;i++)
  {
    if(i==0 || i==1 || isPower(i))
    {
      if(isPower(i)){gl++;}
      else{gs++;}
      lseek(fd,BASE_OFFSET+group_size*gl+group_size1*gs,SEEK_SET);
      write(fd,&super,sizeof(super));
    }
  }
}

void writeBlocktoGd(int fd,struct ext2_group_desc group,int index,struct ext2_super_block super)
{
  int n = ceil(super.s_blocks_count/super.s_blocks_per_group);
  int group_size=(3*n+3)*EXT2_BLOCK_SIZE;
  int group_size1=(2*n+2)*EXT2_BLOCK_SIZE;
  int gl=0;int gs=0;
  for(int i=0;i<n;i++)
  {
    if(i==0 || i==1 || isPower(i))
    {
      if(isPower(i)){gl++;}
      else{gs++;}
      lseek(fd,BASE_OFFSET+group_size*gl+group_size1*gs+EXT2_BLOCK_SIZE*(index+1),SEEK_SET);
      write(fd,&group,sizeof(group));
    }
  }
}
void write_sgd_mod(int fd,struct ext2_group_desc group,int index,struct ext2_super_block super)
{
  writeBlocktoGd(fd,group,index,super);
  writeBlocktoSuper(fd,super);
}

int main(int argc, char *argv[])
{
    struct ext2_super_block super;
    struct ext2_group_desc group;
    struct ext2_inode inode;
    unsigned int fd;
    int my_len;
    int path_flag = 0;
    unsigned int inode_number;
    int group_number;
    int my_group_offset;
    int inode_offset;
    int my_inode_offset_from_beginning;
    string my_destination = argv[3];
    my_len = my_destination.size();
    for(int i = 0; i < my_len; ++i){
    	if(isdigit(my_destination[i]) == FALSE){
    		path_flag = 1;
    	}
    }
    if(path_flag == 0){
    	//inode number is given
		inode_number = atoi(argv[3]);
		fd = open(argv[1], O_RDWR);
	    //read super group
	    lseek(fd,BASE_OFFSET,SEEK_SET);
	    read(fd, &super, sizeof(super));
			std::cout <<"***********"<< super.s_first_ino << '\n';
	    //changed group number calculation!
	    group_number = floor((inode_number-1)/super.s_inodes_per_group);
      int group_desc_offset = BASE_OFFSET+sizeof(super)+sizeof(group)*group_number;

	    lseek(fd,BASE_OFFSET+sizeof(super)+sizeof(group)*group_number,SEEK_SET);
	    read(fd, &group, sizeof(group));
      cout<<"group_number:"<<group_number<<endl;
      cout<<"//////"<<group.bg_free_inodes_count<<endl;

	    //goto my corresponding group and add group.bg_inode_table
	    if(inode_number%super.s_inodes_per_group == 0){
	    	inode_offset = super.s_inodes_per_group - 1;
	    }
	    else{
		    inode_offset = inode_number%super.s_inodes_per_group - 1;
	    }
		//get the inode
    cout<<"//////"<<group.bg_free_inodes_count<<endl;
																						//HOOOOOOOOOOOOPPPPPPPPPPPPP
		  int my_inode_offset_from_fd =  (group.bg_inode_table)*EXT2_BLOCK_SIZE + sizeof(inode)*(inode_offset-40);
	    lseek(fd,my_inode_offset_from_fd,SEEK_SET); //beginning of my desired inode
    	read(fd, &inode, sizeof(inode));
    	unsigned int* my_data_block_address_array = inode.i_block;
      int ibi = 0;
      int empty_fl=0;
      unsigned int in;
      unsigned short rl;
      unsigned char nl,ft;
      int total_read_size=0;
			int total_read_size1;
      string name_of_dir=argv[2];
			cout<<name_of_dir<<"|"<<name_of_dir.size()<<endl;
      int desired_dir_size=sizeof(unsigned int)+sizeof(unsigned short)+sizeof(unsigned char)*2+name_of_dir.size();
      unsigned short tw_rec_len=desired_dir_size;
      int dir_in_block=0;
			unsigned short tmp_rec_len;
			unsigned int inot;
      for(ibi=0;ibi<12;ibi++){
      if(empty_fl==1){break;}
			cout<<ibi<<"||||||||||"<<inode.i_block[ibi]<<endl;
      lseek(fd,(my_data_block_address_array[ibi])*EXT2_BLOCK_SIZE,SEEK_SET);
			int y=0;
      while(1)
      {

        read(fd,&in,sizeof(in));
        if(in==0)
        {cout<<"&&&&&&&&"<<endl;
          empty_fl=1;break;
        }
        else
        {

          lseek(fd,(my_data_block_address_array[ibi])*EXT2_BLOCK_SIZE+total_read_size,SEEK_SET);
          read(fd,&inot,sizeof(unsigned int));
          read(fd,&tmp_rec_len,sizeof(unsigned short));
					//std::cout <<y<<"/* message */"<<tmp_rec_len<<"/*/"<<inot<< '\n';

					if(tmp_rec_len==1024-total_read_size)
					{
						cout<<"REMAINING LEN"<<tmp_rec_len<<endl;
						unsigned char my_name_len;
						lseek(fd,(my_data_block_address_array[ibi])*EXT2_BLOCK_SIZE+total_read_size+sizeof(unsigned int)+sizeof(unsigned short),SEEK_SET);
						read(fd,&my_name_len,sizeof(unsigned char));
						cout<<"act rec partition"<<sizeof(unsigned int)<<"/"<<sizeof(unsigned short)<<"/"<<sizeof(unsigned char)*2<<"/"<<(unsigned int)my_name_len<<endl;
						unsigned short act_rec_len=sizeof(unsigned int)+sizeof(unsigned short)+sizeof(unsigned char)*2+my_name_len*sizeof(char);
						lseek(fd,(my_data_block_address_array[ibi])*EXT2_BLOCK_SIZE+total_read_size+sizeof(unsigned int),SEEK_SET);
						write(fd,&act_rec_len,sizeof(unsigned short));
						lseek(fd,sizeof(unsigned char)*2+my_name_len*sizeof(char),SEEK_CUR);
						tmp_rec_len=act_rec_len;
						cout << "act_rec_len: " << act_rec_len << endl;
					}
					total_read_size+=tmp_rec_len;
					std::cout <<y<<"/* message */"<<tmp_rec_len<<"/*/"<<inot<< '\n';y++;

        }
        if(1024-total_read_size<desired_dir_size)
        {cout<<"aaaaaaaaaaaa"<<endl;
          break;
        }

      }
			cout<<"^^^^^^^"<<total_read_size<<endl;
			total_read_size1=total_read_size;
      total_read_size=0;

    }
    ibi--;
    cout<<"//////"<<group.bg_free_inodes_count<<endl;

    lseek(fd,(group.bg_inode_bitmap-1)*EXT2_BLOCK_SIZE+12,SEEK_SET);
    cout<<"//////"<<group.bg_free_inodes_count<<endl;

    char inode_bitmap_index;
    int i=0;int bmf=0;
    int bitmap_byte_index,bitmap_byte_offset_index;
    std::cout << "-----------" << '\n';
    while(i<1012)
    {
      read(fd,&inode_bitmap_index,sizeof(char));
      for(int k=0;k<8;k++)
      {
        cout<<((inode_bitmap_index&(1<<(7-k)))>>(7-k));
      }
      cout<<endl;
      std::cout << "-----------" << '\n';
      for(int j=0;j<8;j++)
      {

        if(!(inode_bitmap_index>>j & 1))//CHECK LATER!!!!!!!!!
        {
          char t1;
          //CHECK IF INODE BITMAP MODIFIED
          lseek(fd,(group.bg_inode_bitmap-1)*EXT2_BLOCK_SIZE+12+i,SEEK_SET);
          read(fd,&t1,sizeof(char));
          cout<<"||"<<(t1&(1<<j))<<"||" <<(char)0<<"||*||"<<(inode_bitmap_index&(1<<j))<<endl;
          //END CHECK
          inode_bitmap_index=inode_bitmap_index | (1<<j);
          lseek(fd,(group.bg_inode_bitmap-1)*EXT2_BLOCK_SIZE+12+i,SEEK_SET);
          write(fd,&inode_bitmap_index,sizeof(char));
          //CHECK IF INODE BITMAP MODIFIED
          lseek(fd,(group.bg_inode_bitmap-1)*EXT2_BLOCK_SIZE+12+i,SEEK_SET);
          read(fd,&t1,sizeof(char));
          cout<<"||"<<j<<"||" <<(char)0<<"||*||"<<(inode_bitmap_index&(1<<j))<<endl;
          //END CHECK
          unsigned short ninc,ninc_test;
          cout<<"//0////"<<group.bg_free_inodes_count<<endl;

          ninc=group.bg_free_inodes_count;

          cout<<"//1////"<<group.bg_free_inodes_count<<endl;
          cout<<"!!!!"<<ninc<<endl;

          ninc--;

          lseek(fd,group_desc_offset+(sizeof(unsigned int)*3)+sizeof(unsigned short),SEEK_SET);
          write(fd,&ninc,sizeof(unsigned short));
          lseek(fd,group_desc_offset+(sizeof(unsigned int)*3)+sizeof(unsigned short),SEEK_SET);
          read(fd,&ninc,sizeof(unsigned short));

          cout<<"!!1!!"<<ninc<<endl;

          lseek(fd,BASE_OFFSET+sizeof(super)+sizeof(group)*group_number,SEEK_SET);
    	    read(fd, &group, sizeof(group));

          bitmap_byte_index=i;
          bitmap_byte_offset_index=j;
          bmf=1;break;
        }
      }
      if(bmf==1)
      {
        break;
      }
      i++;
    }
    cout<<"//////"<<group.bg_free_inodes_count<<endl;

    int inode_block_number=8*bitmap_byte_index+bitmap_byte_offset_index+1;
    ft=2;
    //TEST DIR ENTRY INODE Number
    cout<<"1:"<<inode_block_number<<endl;
    unsigned int dint;
    lseek(fd,(my_data_block_address_array[ibi])*EXT2_BLOCK_SIZE+total_read_size1,SEEK_SET);
    read(fd,&dint,sizeof(unsigned int));
    cout<<"2:"<<dint<<endl;
    //END DIR ENTRY TEST
		nl=(unsigned char)name_of_dir.size();
		int temp_act_rec = tw_rec_len;
		cout<<"/////////"<<total_read_size1<<endl;
		tw_rec_len = (unsigned short)(1024 - total_read_size1);
		cout<<"!!!!!!!!!"<<tw_rec_len<<endl;
    lseek(fd,(my_data_block_address_array[ibi])*EXT2_BLOCK_SIZE+total_read_size1,SEEK_SET);
    write(fd,&inode_block_number,sizeof(unsigned int));
    write(fd,&tw_rec_len,sizeof(unsigned short));
    write(fd,&nl,sizeof(unsigned char));
		lseek(fd,(my_data_block_address_array[ibi])*EXT2_BLOCK_SIZE+total_read_size1+sizeof(unsigned int)+sizeof(unsigned short),SEEK_SET);
		read(fd,&nl,sizeof(unsigned char));
		cout<<"safdasfd "<<(unsigned int)nl<<endl;

    write(fd,&ft,sizeof(unsigned char));
    lseek(fd,(my_data_block_address_array[ibi])*EXT2_BLOCK_SIZE+total_read_size1,SEEK_SET);
    read(fd,&dint,sizeof(unsigned int));
    cout<<"3:"<<dint<<endl;
		lseek(fd,(my_data_block_address_array[ibi])*EXT2_BLOCK_SIZE+total_read_size1+sizeof(unsigned int),SEEK_SET);
		read(fd,&tw_rec_len,sizeof(unsigned short));
		cout << "tw_rec_len_at_last: " << tw_rec_len << endl;
    char char_tw;
    for(int o=0;o<name_of_dir.size();o++)
    {
      char_tw=name_of_dir[o];
			cout<<char_tw;
      write(fd,&char_tw,sizeof(char));
    }
		cout<<endl;
    inode_block_number--;
    lseek(fd,group.bg_inode_table+inode_block_number*EXT2_BLOCK_SIZE,SEEK_SET);
    struct stat *statbuf =new struct stat;
    stat(argv[1],statbuf);
    unsigned short new_mode = statbuf -> st_mode;
    unsigned short new_uid = statbuf -> st_uid & 0xFFFF;
    unsigned int new_size = statbuf -> st_size;
    unsigned int new_atime = statbuf -> st_atim.tv_sec;
    unsigned int new_ctime = statbuf -> st_ctim.tv_sec;
    unsigned int new_mtime = statbuf -> st_mtim.tv_sec;
    unsigned short new_gid = statbuf -> st_gid & 0xFFFF;
    unsigned short new_links_count = statbuf -> st_nlink;
    unsigned int new_blocks = statbuf -> st_blocks/2;
    write(fd,&new_mode,sizeof(unsigned short));
    write(fd,&new_uid,sizeof(unsigned short));
    write(fd,&new_size,sizeof(unsigned int));
    write(fd,&new_atime,sizeof(unsigned int));
    write(fd,&new_ctime,sizeof(unsigned int));
    write(fd,&new_mtime,sizeof(unsigned int));
    write(fd,&new_gid,sizeof(unsigned short));
    write(fd,&new_links_count,sizeof(unsigned short));
    write(fd,&new_blocks,sizeof(unsigned int));







  }
    else
    {

    }
}
/*
struct stat {
              dev_t     st_dev;         /* ID of device containing file
              ino_t     st_ino;         /* Inode number
              mode_t    st_mode;        /* File type and mode
              nlink_t   st_nlink;       /* Number of hard links
              uid_t     st_uid;         /* User ID of owner
              gid_t     st_gid;         /* Group ID of owner
              dev_t     st_rdev;        /* Device ID (if special file)
              off_t     st_size;        /* Total size, in bytes
              blksize_t st_blksize;     /* Block size for filesystem I/O
              blkcnt_t  st_blocks;      /* Number of 512B blocks allocated

              /* Since Linux 2.6, the kernel supports nanosecond
                 precision for the following timestamp fields.
                 For the details before Linux 2.6, see NOTES.

              struct timespec st_atim;  /* Time of last access
              struct timespec st_mtim;  /* Time of last modification
              struct timespec st_ctim;  /* Time of last status change

          #define st_atime st_atim.tv_sec      /* Backward compatibility
          #define st_mtime st_mtim.tv_sec
          #define st_ctime st_ctim.tv_sec
        };*/







      //data block addresses are absolute block addresses
    	/*int aai = 0;
      while(my_data_block_address_array[aai]!=0)
      {
        aai++;
      }

      int my_bboff =  (group.bg_block_bitmap-1)*EXT2_BLOCK_SIZE ;
      int n=ceil(super.s_blocks_count/super.s_blocks_per_group);

      int i=2n+3;
      char a;int bmf=0;
      while(i<1024)
      {
        lseek(fd,my_bboff+i,SEEK_SET);
        read (fd,&a,sizeof(a));
        for(int j=0;j<8;j++)
        {
          if(!(a>>j & 1))
          {
            bitmap_byte_index=i;
            bitmap_byte_offset_index=j;
            bmf=1;break;
          }
        }
        if(bmf==1)
        {
          break;
        }
      }
      int data_block_index=8*bitmap_byte_index+bitmap_byte_offset_index;
      my_group_offset = BASE_OFFSET+(super.s_blocks_per_group*EXT2_BLOCK_SIZE*group_number);
      cout<<fd<<"!!!!!!!!!!!!!!"<<endl;
      lseek(fd,(2*n+3+data_block_index)*EXT2_BLOCK_SIZE+my_group_offset,SEEK_SET);
      inode.i_block[aai]=(2*n+3+data_block_index)*EXT2_BLOCK_SIZE+my_group_offset;
      struct ext2_dir_entry dir;*/










      /*vector<int> empty_data_block_index;
    	int my_first_empty_data_block_index = -1;
    	//look for directs
      int indirect_flag=0;
    	while(1){
    		if(i == 12){
    			break;
    		}
    		if(my_data_block_address_array[i] == 0){
    			empty_data_block_index.push_back(i);
    		}
    		i++;
    	}
    	if(empty_data_block_index.empty()){
    		//there are no direct block addresses empty left
    	}*/
    	//inode contains my target inode struct
    	//it is a DIRECTORY
    	//i have to write the source file to that DIRECTORY
    	//data blocks of an inode which mode is DIRECTORY has dir_entry struct in them
    	//they can be files or directories, actually i we do not care about that
    	//now, we have to find an empty cell at i_block[12]

		//struct ext2_inode {
        //unsigned short i_mode;        /* File mode */
        //unsigned short i_uid;         /* Low 16 bits of Owner Uid */
        //unsigned int   i_size;        /* Size in bytes */
        //unsigned int   i_atime;       /* Access time */
        //unsigned int   i_ctime;       /* Creation time */
        //unsigned int   i_mtime;       /* Modification time */
        //unsigned int   i_dtime;       /* Deletion Time */
        //unsigned short i_gid;         /* Low 16 bits of Group Id */
        //unsigned short i_links_count; /* Links count */
        //unsigned int   i_blocks;      /* Blocks count IN DISK SECTORS*/
        //unsigned int   i_flags;       /* File flags */
        //unsigned int   osd1;          /* OS dependent 1 */
        //unsigned int   i_block[15];   /* Pointers to blocks */
        //unsigned int   i_generation;  /* File version (for NFS) */
        //unsigned int   i_file_acl;    /* File ACL */
        //unsigned int   i_dir_acl;     /* Directory ACL */
        //unsigned int   i_faddr;       /* Fragment address */
        //unsigned int   extra[3];
		//};
