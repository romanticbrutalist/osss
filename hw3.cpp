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
#include <vector>
#define TRUE 1
#define FALSE 0

#define BASE_OFFSET 1024
//#define BLOCK_SIZE 1024
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


vector<unsigned int> find_empty_data_block(int fd,int n,struct ext2_group_desc group,int group_n,struct ext2_super_block super)
{
	unsigned short binc;
	struct ext2_group_desc g;
	int BLOCK_SIZE=(1<<(10+super.s_log_block_size));
	lseek(fd,BASE_OFFSET+sizeof(super),SEEK_SET);
	read(fd,&g,sizeof(group));
	int ng = ceil(super.s_blocks_count/super.s_blocks_per_group);
	//std:://cout << "asd"<<ng << '\n';
	vector<unsigned int> block_id_vec;
	lseek(fd,(g.bg_block_bitmap)*BLOCK_SIZE,SEEK_SET);
	char bb_8;
	binc=g.bg_free_blocks_count;

	int f=0;
		for(int i=0;i<BLOCK_SIZE;i++)
		{
			read(fd,&bb_8,sizeof(char));
			for(int j=0;j<8;j++)
			{
				if(!(bb_8&(1<<j)))
				{
					lseek(fd,(g.bg_block_bitmap)*BLOCK_SIZE+i,SEEK_SET);
					bb_8 = bb_8 | 1<<j;
					write(fd,&bb_8,sizeof(char));
					block_id_vec.push_back((unsigned int)(group_n*super.s_blocks_per_group+8*i+j+super.s_first_data_block));

					//cout<<"//1////"<<group.bg_free_inodes_count<<endl;
					//cout<<"!!!!"<<ninc<<endl;
					//cout<<"free block count: "<<binc<<endl;
					binc--;




					if(block_id_vec.size()>=n){//std:://cout << block_id_vec.size()<<"|"<<n << '\n';
					f=1;break;
				}
				}

			}
			if(f==1){break;}
		}
		lseek(fd,BASE_OFFSET+sizeof(super)+(sizeof(unsigned int)*3),SEEK_SET);
		write(fd,&binc,sizeof(unsigned short));
		lseek(fd,BASE_OFFSET+sizeof(super)+(sizeof(unsigned int)*3),SEEK_SET);
		read(fd,&binc,sizeof(unsigned short));
		//cout<<group_n<<": free block count: "<<binc<<endl;
		int cgn=0;
		group_n=0;
		while((block_id_vec.size()<n))
		{
				if(ng==group_n){group_n=-1;}
				group_n++;
				if(cgn==group_n){break;}
				lseek(fd,BASE_OFFSET+sizeof(super)+sizeof(group)*group_n,SEEK_SET);
				read(fd,&g,sizeof(group));
				lseek(fd,(g.bg_block_bitmap)*BLOCK_SIZE,SEEK_SET);
				char b_8;
				f=0;
				binc=g.bg_free_blocks_count;

					for(int i=0;i<BLOCK_SIZE;i++)
					{
						read(fd,&b_8,sizeof(char));
						for(int j=0;j<8;j++)
						{
							if(!(b_8& (1<<j)))
							{
								lseek(fd,(g.bg_block_bitmap)*BLOCK_SIZE+i,SEEK_SET);
								b_8 = b_8 | 1<<j;
								write(fd,&b_8,sizeof(char));
								//cout<<"BLOCK BITMAP INDEX"<<8*i+j+1<<endl;
								block_id_vec.push_back((unsigned int)(group_n*super.s_blocks_per_group+8*i+j+super.s_first_data_block));
								//binc=g.bg_free_blocks_count;

								//cout<<"//1////"<<group.bg_free_inodes_count<<endl;
								//cout<<"!!!!"<<ninc<<endl;
								//cout<<group_n<<": //free block count: "<<binc<<endl;
								binc=binc-1;


								if(block_id_vec.size()>=n){//std:://cout << block_id_vec.size()<<"|"<<n << '\n';
								f=1;break;}
							}

						}
						if(f==1){break;}
					}
					//cout<<"SON BINC"<<binc<<endl;
					lseek(fd,BASE_OFFSET+sizeof(super)+(sizeof(unsigned int)*3+sizeof(group)*group_n),SEEK_SET);
					write(fd,&binc,sizeof(unsigned short));
					lseek(fd,BASE_OFFSET+sizeof(super)+(sizeof(unsigned int)*3+sizeof(group)*group_n),SEEK_SET);
					read(fd,&binc,sizeof(unsigned short));
					//cout<<"FREE BLOCK COUNT: "<<binc<<endl;

		}

		//cout<<"//0////"<<group.bg_free_inodes_count<<endl;
		unsigned int sfb;
		lseek(fd,BASE_OFFSET+(sizeof(int)*3),SEEK_SET);
		read(fd,&sfb,sizeof(unsigned short));
		//cout<<"super free count: "<<sfb<<endl;
		sfb=sfb-block_id_vec.size();
		lseek(fd,BASE_OFFSET+(sizeof(int)*3),SEEK_SET);
		write(fd,&sfb,sizeof(unsigned short));
		lseek(fd,BASE_OFFSET+(sizeof(int)*3),SEEK_SET);
		read(fd,&sfb,sizeof(unsigned short));
		//cout<<"super free count: "<<sfb<<endl;




	return block_id_vec;

}
int inninsert(char *argv[],int given_in)
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

		//inode number is given
	inode_number = given_in;
	fd = open(argv[1], O_RDWR);
		//read super group
		lseek(fd,BASE_OFFSET,SEEK_SET);
		read(fd, &super, sizeof(super));
		int BLOCK_SIZE=(1<<(10+super.s_log_block_size));
		//std:://cout <<"***********"<< super.s_first_ino << '\n';
		//changed group number calculation!
		group_number = floor((inode_number-1)/super.s_inodes_per_group);
		int group_desc_offset = BASE_OFFSET+sizeof(super)+sizeof(group)*group_number;

		lseek(fd,BASE_OFFSET+sizeof(super)+sizeof(group)*group_number,SEEK_SET);
		read(fd, &group, sizeof(group));
		//if(group.bg_free_inodes_count==0){return 0;}
		//cout<<"group_number:"<<group_number<<endl;
		//cout<<"//////"<<group.bg_free_inodes_count<<endl;

		//goto my corresponding group and add group.bg_inode_table
		inode_offset=inode_number%super.s_inodes_per_group-1;



		//get the inode
		//cout<<"//////"<<inode_offset<<endl;
																					//HOOOOOOOOOOOOPPPPPPPPPPPPP
		int my_inode_offset_from_fd =  (group.bg_inode_table)*BLOCK_SIZE + (super.s_inode_size)*(inode_offset);
		lseek(fd,my_inode_offset_from_fd,SEEK_SET); //beginning of my desired inode
		read(fd, &inode, (super.s_inode_size));
		//cout<<"i_links_count: "<<inode.i_links_count<<endl;

		//inode.i_links_count=1;
		//lseek(fd,my_inode_offset_from_fd,SEEK_SET); //beginning of my desired inode
		//write(fd, &inode, (super.s_inode_size));
		//lseek(fd,my_inode_offset_from_fd,SEEK_SET); //beginning of my desired inode
		//read(fd, &inode, (super.s_inode_size));
		//cout<<"i_links_count: "<<inode.i_links_count<<endl;
		//lseek(fd,118*BLOCK_SIZE,SEEK_SET); //beginning of my desired inode
		//read(fd, &inode, (super.s_inode_size));
		unsigned int* my_data_block_address_array = inode.i_block;
		for(int k=0;k<12;k++)
		{
			//std::cout << my_data_block_address_array[k] << '\n';
		}
		int ibi = 0;
		int empty_fl=0;
		unsigned int in;
		unsigned short rl;
		unsigned char nl,ft;
		int total_read_size=0;
		int total_read_size1;
		string name_of_dir=argv[2];
		//cout<<name_of_dir<<"|"<<name_of_dir.size()<<endl;
		int desired_dir_size=sizeof(unsigned int)+sizeof(unsigned short)+sizeof(unsigned char)*2+name_of_dir.size();
		unsigned short tw_rec_len=desired_dir_size;
		int dir_in_block=0;
		unsigned short tmp_rec_len;
		unsigned int inot;
		for(ibi=0;ibi<12;ibi++){
		if(empty_fl==1){break;}
		//cout<<ibi<<"||||||||||"<<inode.i_block[ibi]<<endl;
		lseek(fd,(my_data_block_address_array[ibi])*BLOCK_SIZE,SEEK_SET);
		int y=0;
		unsigned char my_name_len;
		char tt[255];
		for(int u=0;u<255;u++)
		{
			tt[u]='\0';
		}
		while(1)
		{
			//std::cout << "==============================================================" << '\n';
			lseek(fd,(my_data_block_address_array[ibi])*BLOCK_SIZE+total_read_size,SEEK_SET);
			read(fd,&in,sizeof(unsigned int));
			//cout<<"1/direntry inode numbers: "<<in<<endl;
			if(in==0)
			{
				//cout<<"&&&&&&&&"<<endl;
				empty_fl=1;break;
			}
			else
			{
				//read(fd,&inot,sizeof(unsigned int));
				read(fd,&tmp_rec_len,sizeof(unsigned short));
				read(fd,&my_name_len,sizeof(unsigned char));
				//cout<<"2/name len: "<<(int)my_name_len<<endl;
				lseek(fd,sizeof(unsigned char),SEEK_CUR);
				read(fd,&tt,sizeof(char)*(int)my_name_len);
				//cout<<"3/"<<tt<<endl;
				//std::cout <<"4/"<<y<<"/* message */"<<tmp_rec_len<<"/*/"<<in<< '\n';
				//cout<<"5/total read|tmp_rec_len"<<total_read_size<<"|"<<tmp_rec_len<<endl;
				if(tmp_rec_len==BLOCK_SIZE-total_read_size)
				{
					//std::cout << "FOUND FINAL DIR_ENTRY" << '\n';
					//cout<<"REMAINING LEN"<<tmp_rec_len<<endl;
					//cout<<"inode iblock: "<<my_data_block_address_array[ibi]<<endl;
					lseek(fd,(my_data_block_address_array[ibi])*BLOCK_SIZE+total_read_size+sizeof(unsigned int)+sizeof(unsigned short),SEEK_SET);
					read(fd,&my_name_len,sizeof(unsigned char));
					//cout<<"final name len: "<<(int)my_name_len<<endl;
					lseek(fd,sizeof(unsigned char),SEEK_CUR);
					string st;
					read(fd,&tt,sizeof(char)*(int)my_name_len);
					//std:://cout <<(int)my_name_len<<"/* message */" << '\n';
					//string st(tt);
					//std:://cout << "/* message */" << '\n';
					//std::cout << tt << '\n';
					//cout<<"act rec partition"<<sizeof(unsigned int)<<"/"<<sizeof(unsigned short)<<"/"<<sizeof(unsigned char)*2<<"/"<<(unsigned int)my_name_len<<endl;
					unsigned short act_rec_len=sizeof(unsigned int)+sizeof(unsigned short)+sizeof(unsigned char)*2+(int)my_name_len*sizeof(char);
					if(!(act_rec_len%4==0))
					{
						act_rec_len=4-act_rec_len%4+act_rec_len;
					}
					lseek(fd,(my_data_block_address_array[ibi])*BLOCK_SIZE+total_read_size+sizeof(unsigned int),SEEK_SET);
					write(fd,&act_rec_len,sizeof(unsigned short));
					lseek(fd,sizeof(unsigned char)*2+(int)my_name_len*sizeof(char),SEEK_CUR);
					tmp_rec_len=act_rec_len;
					//cout << "act_rec_len: " << act_rec_len << endl;
				}
				total_read_size+=tmp_rec_len;
				//std:://cout <<y<<"/* message */"<<tmp_rec_len<<"/*/"<<inot<< '\n';y++;

			}
			if(BLOCK_SIZE-total_read_size<desired_dir_size)
			{//cout<<"aaaaaaaaaaaa"<<endl;
				break;
			}
		}
		//cout<<"^^^^^^^"<<total_read_size<<endl;
		total_read_size1=total_read_size;
		total_read_size=0;
	}
	ibi--;

	//cout<<"//////"<<group.bg_free_inodes_count<<endl;
	//cout<<"//////"<<group.bg_free_inodes_count<<endl;

	char inode_bitmap_index;
	int i=0;int bmf=0;
	int bitmap_byte_index,bitmap_byte_offset_index;
	struct ext2_group_desc ing;
	//std:://cout << "-----------" << '\n';
	int inode_flag=0;
	int gi=0;
	while(1)
	{
		lseek(fd,BASE_OFFSET+sizeof(super)+sizeof(group)*gi,SEEK_SET);
		read(fd,&ing,sizeof(group));
		if(ing.bg_free_inodes_count==0)
		{
			gi++;
			continue;
		}
		else
		{
			break;
		}

	}
	lseek(fd,BASE_OFFSET+sizeof(super)+sizeof(group)*gi,SEEK_SET);
	read(fd,&ing,sizeof(group));
	lseek(fd,(ing.bg_inode_bitmap)*BLOCK_SIZE,SEEK_SET);
	//cout<<gi<<"free inode gn: "<<ing.bg_free_inodes_count<<endl;
	while(i<BLOCK_SIZE)
	{	//cout<<i<<": ";
		read(fd,&inode_bitmap_index,sizeof(char));
		//std::cout << "+++++++++++" << '\n';
	
		//cout<<endl;
		//std::cout << "++++++++++++" << '\n';
		for(int j=0;j<8;j++)
		{

			if(!(inode_bitmap_index & (1 << j)))//CHECK LATER!!!!!!!!!
			{
				char t1;
				//CHECK IF INODE BITMAP MODIFIED
				lseek(fd,(ing.bg_inode_bitmap)*BLOCK_SIZE+i,SEEK_SET);
				read(fd,&t1,sizeof(char));
				//cout<<"||"<<(t1&(1<<j))<<"||" <<(char)0<<"||*||"<<(inode_bitmap_index&(1<<j))<<endl;
				//END CHECK
				inode_bitmap_index=inode_bitmap_index | (1<<j);
				lseek(fd,(ing.bg_inode_bitmap)*BLOCK_SIZE+i,SEEK_SET);
				write(fd,&inode_bitmap_index,sizeof(char));
				//CHECK IF INODE BITMAP MODIFIED
				lseek(fd,(ing.bg_inode_bitmap)*BLOCK_SIZE+i,SEEK_SET);
				read(fd,&t1,sizeof(char));
				//cout<<"||"<<j<<"||" <<(char)0<<"||*||"<<(inode_bitmap_index&(1<<j))<<endl;
				//END CHECK
				unsigned short ninc,ninc_test;
				//cout<<"//0////"<<group.bg_free_inodes_count<<endl;

				ninc=ing.bg_free_inodes_count;

				//cout<<"//1////"<<group.bg_free_inodes_count<<endl;
				//cout<<"!!!!"<<ninc<<endl;

				ninc--;

				lseek(fd,BASE_OFFSET+sizeof(super)+sizeof(group)*gi+(sizeof(unsigned int)*3)+sizeof(unsigned short),SEEK_SET);
				write(fd,&ninc,sizeof(unsigned short));
				lseek(fd,BASE_OFFSET+sizeof(super)+sizeof(group)*gi+(sizeof(unsigned int)*3)+sizeof(unsigned short),SEEK_SET);
				read(fd,&ninc,sizeof(unsigned short));

				//cout<<"!!1!!"<<ninc<<endl;
				unsigned int ifb;
				lseek(fd,BASE_OFFSET+(sizeof(int)*4),SEEK_SET);
				read(fd,&ifb,sizeof(unsigned int));
				//cout<<"super free count: "<<ifb<<endl;
				ifb--;
				lseek(fd,BASE_OFFSET+(sizeof(int)*4),SEEK_SET);
				write(fd,&ifb,sizeof(unsigned int));
				lseek(fd,BASE_OFFSET+(sizeof(int)*4),SEEK_SET);
				read(fd,&ifb,sizeof(unsigned int));
				//cout<<"super free count: "<<ifb<<endl;


				bitmap_byte_index=i;
				bitmap_byte_offset_index=j;
				//cout<<"HOOOOOOPPPPPPPPP"<<endl;
				bmf=1;break;
			}
		}
		//cout<<i<<": ";
		lseek(fd,(ing.bg_inode_bitmap)*BLOCK_SIZE+i,SEEK_SET);
			//std::cout << "------------" << '\n';
			read(fd,&inode_bitmap_index,sizeof(char));
			for(int k=0;k<8;k++)
			{
				//cout<<((inode_bitmap_index&(1<<k))>>k);
			}
			//cout<<endl;
			//std::cout << "-----------" << '\n';
		if(bmf==1)
		{
			break;
		}
		i++;
	}
	//cout<<"//////"<<group.bg_free_inodes_count<<endl;
//	cout<<"BITMAP INDEXES: "<<bitmap_byte_index<<"|"<<bitmap_byte_offset_index<<endl;
	int inode_block_number=8*bitmap_byte_index+bitmap_byte_offset_index+1;
	//std::cout <<"inode bitmap index: "<<(int)inode_block_number << '\n';

	ft=1;
	//TEST DIR ENTRY INODE Number
	//cout<<"1:"<<inode_block_number<<endl;
	unsigned int dint;
	//lseek(fd,(my_data_block_address_array[ibi])*BLOCK_SIZE+total_read_size1,SEEK_SET);
	//read(fd,&dint,sizeof(unsigned int));
	//cout<<"2:"<<dint<<endl;
	//END DIR ENTRY TEST
	nl=(unsigned char)name_of_dir.size();
	int temp_act_rec = tw_rec_len;
	//cout<<"/////////"<<total_read_size1<<endl;
	tw_rec_len = (unsigned short)(BLOCK_SIZE - total_read_size1);
	int relative_inode_number=inode_block_number;
	inode_block_number=inode_block_number+super.s_inodes_per_group*gi;
	dint=(unsigned int)inode_block_number;
	//cout<<dint<<"INODE BLOCK NUMBER TO WRITE:"<<inode_block_number<<endl;
	//cout<<total_read_size1<<"!!!!!!!!!"<<tw_rec_len<<endl;
	lseek(fd,(my_data_block_address_array[ibi])*BLOCK_SIZE+total_read_size1,SEEK_SET);
	write(fd,&dint,sizeof(unsigned int));
	write(fd,&tw_rec_len,sizeof(unsigned short));
	write(fd,&nl,sizeof(unsigned char));
	//lseek(fd,(my_data_block_address_array[ibi])*BLOCK_SIZE+total_read_size1+sizeof(unsigned int)+sizeof(unsigned short),SEEK_SET);
	//read(fd,&nl,sizeof(unsigned char));
	//cout<<"safdasfd "<<(unsigned int)nl<<endl;

	write(fd,&ft,sizeof(unsigned char));

	//cout<<"3:"<<dint<<endl;
	lseek(fd,(my_data_block_address_array[ibi])*BLOCK_SIZE+total_read_size1+sizeof(unsigned int),SEEK_SET);
	read(fd,&tw_rec_len,sizeof(unsigned short));
	//cout << "tw_rec_len_at_last: " << tw_rec_len << endl;
	char char_tw;
	lseek(fd,2*sizeof(unsigned char),SEEK_CUR);
	for(int o=0;o<name_of_dir.size();o++)
	{
		char_tw=name_of_dir[o];
		//cout<<char_tw;
		write(fd,&char_tw,sizeof(char));
	}
	//cout<<"+";
	//cout<<endl;
	//std:://cout << "/* message */" << '\n';
	//inode_block_number--;
	//std:://cout << "/* message */" << '\n';
	lseek(fd,ing.bg_inode_table*BLOCK_SIZE+(relative_inode_number-1)*(super.s_inode_size),SEEK_SET);
	//std:://cout << "/* message */" << '\n';
	struct stat statbuf;
	//std:://cout << "/* message */" << '\n';
	stat(argv[2],&statbuf);
	unsigned short new_mode = statbuf.st_mode;
	//std:://cout << "/* message */" << '\n';
	unsigned short new_uid = statbuf.st_uid & 0xFFFF;
	unsigned int new_size = statbuf.st_size;
	unsigned int new_atime = statbuf.st_atim.tv_sec;
	unsigned int new_ctime = statbuf.st_ctim.tv_sec;
	unsigned int new_mtime = statbuf.st_mtim.tv_sec;
	unsigned short new_gid = statbuf.st_gid & 0xFFFF;
	unsigned short new_links_count = 1;
	unsigned int new_blocks = statbuf.st_blocks/2;
	write(fd,&new_mode,sizeof(unsigned short));
	write(fd,&new_uid,sizeof(unsigned short));
	write(fd,&new_size,sizeof(unsigned int));
	write(fd,&new_atime,sizeof(unsigned int));
	write(fd,&new_ctime,sizeof(unsigned int));
	write(fd,&new_mtime,sizeof(unsigned int));
	lseek(fd,sizeof(unsigned int),SEEK_CUR);
	write(fd,&new_gid,sizeof(unsigned short));
	write(fd,&new_links_count,sizeof(unsigned short));
	write(fd,&new_blocks,sizeof(unsigned int));

	struct ext2_inode inodedir;
	lseek(fd,ing.bg_inode_table*BLOCK_SIZE+(relative_inode_number-1)*(super.s_inode_size),SEEK_SET);
	read(fd,&inodedir,(super.s_inode_size));
	unsigned int idirsize = inodedir.i_size;
	int nofb;
	int off;

	if(!(idirsize%BLOCK_SIZE))
	{
		nofb = floor(idirsize/BLOCK_SIZE);
		off=BLOCK_SIZE;
	}
	else
	{

		nofb = floor(idirsize/BLOCK_SIZE)+1;
		off = idirsize-floor(idirsize/BLOCK_SIZE)*BLOCK_SIZE;
	}

	unsigned int r_s;
	r_s=(BLOCK_SIZE/512)*nofb;
	lseek(fd,ing.bg_inode_table*BLOCK_SIZE+(relative_inode_number-1)*(super.s_inode_size)+5*sizeof(unsigned int)+4*sizeof(unsigned short),SEEK_SET);
	write(fd,&r_s,sizeof(unsigned int));



	//std::cout << "REAL SIZE: " <<r_s<< '\n';
	vector<unsigned int> mv;
	struct ext2_group_desc g1;
	lseek(fd,BASE_OFFSET+sizeof(super),SEEK_SET);
	read(fd,&g1,sizeof(group));
	lseek(fd,(g1.bg_block_bitmap)*BLOCK_SIZE ,SEEK_SET);
	char tu;
	i=0;
	while(i<BLOCK_SIZE)
	{
	//cout<<i<<": ";
	read(fd,&tu,sizeof(char));
	for(int k=0;k<8;k++)
	{

			//cout<<8*i+k+1<<": "<<((tu&(1<<(7-k)))>>(7-k))<<endl;

	}
	//cout<<endl;
	i++;
	}
	//std::cout << "==========VEKTÖR========" << '\n';

	mv=find_empty_data_block(fd,nofb,group,group_number,super);
	for(int h=0;h<mv.size();h++)
	{
	//std::cout << mv[h] << ' ';

	}

	//cout<<endl;
	//std::cout << "==========VEKTÖR========" << '\n';
	//std::cout << "relative: " <<relative_inode_number<< '\n';
	i=0;
	lseek(fd,(g1.bg_block_bitmap)*BLOCK_SIZE ,SEEK_SET);
	while(i<BLOCK_SIZE)
	{
	//cout<<i<<": ";
	read(fd,&tu,sizeof(char));
	for(int k=0;k<8;k++)
	{

			//cout<<8*i+k+1<<": "<<((tu&(1<<(7-k)))>>(7-k))<<endl;

	}
	//cout<<endl;
	i++;
	}
	lseek(fd,ing.bg_inode_table*BLOCK_SIZE+(relative_inode_number-1)*(super.s_inode_size)+8*sizeof(unsigned int)+4*sizeof(unsigned short),SEEK_SET);
	unsigned int twb;
	for(int j=0;j<mv.size();j++)
	{
	twb=mv[j];
	write(fd,&twb,sizeof(unsigned int));
	}
	lseek(fd,ing.bg_inode_table*BLOCK_SIZE+(relative_inode_number-1)*(super.s_inode_size),SEEK_SET);
	read(fd,&inodedir,(super.s_inode_size));

	for(int j = 0;j<nofb;j++)
	{
	//cout<<"IBLOCK SET MI AGAAAAA: "<<inodedir.i_block[j]<<endl;
	}
	int fd1 = open(argv[2], O_RDONLY);
	char dtw[BLOCK_SIZE];
	char last[off];

	//CHANGE BITMAPS
	//CHANGE METADATAS
	//CHECK INODE INDEX
	lseek(fd,BASE_OFFSET+sizeof(super),SEEK_SET);
	read(fd,&group,sizeof(group));
	//cout<<"asfdasdf"<<group.bg_inode_table<<endl;
	int rel_block;

	for(int j=0;j<nofb;j++)
	{
	if(j==nofb-1)
	{
		read(fd1,&last,off);
		rel_block=mv[j]%super.s_blocks_per_group;
		////cout<<last<<"|";//<<"**A**";
		//cout<<"YAZIYOZ"<<mv[j]<<endl;
		lseek(fd,(mv[j])*BLOCK_SIZE,SEEK_SET);
		write(fd,&last,off);
		lseek(fd,(mv[j])*BLOCK_SIZE,SEEK_SET);
		read(fd,&last,off);


			//cout<<last<<endl;

		//cout<<"--------------"<<j<<"---------------"<<endl;

	}
	else
	{
		read(fd1,&dtw,BLOCK_SIZE);
		////cout<<dtw<<"|";//<<"**A**";
		//cout<<"YAZIYOZ"<<mv[j]<<endl;

		lseek(fd,(mv[j])*BLOCK_SIZE,SEEK_SET);
		write(fd,&dtw,BLOCK_SIZE);
		lseek(fd,(mv[j])*BLOCK_SIZE,SEEK_SET);
		read(fd,&dtw,BLOCK_SIZE);


			//cout<<dtw<<endl;

		//cout<<"--------------"<<j<<"---------------"<<endl;
	}
	}
	//cout<<endl;
	//cout<<group.bg_block_bitmap<<"|"<<group.bg_inode_bitmap<<endl;
	i=0;
int finali = relative_inode_number+gi*super.s_inodes_per_group;
cout<<finali<<" ";
for(int mal_petek=0;mal_petek<mv.size()-1;mal_petek++)
{
	cout<<mv[mal_petek]<<",";
}
cout<<mv[mv.size()-1]<<endl;

}
int wwp(char *argv[])
{
	struct ext2_super_block super;
	struct ext2_group_desc group,group_find_inode;
	struct ext2_inode inode,inode_find;
	unsigned int fd;
	int my_len;
	int path_flag = 0;
	unsigned int inode_number;
	int group_number;
	int my_group_offset;
	int inode_offset;
	int my_inode_offset_from_beginning;
	vector<string> path;
	string token = "";
	string my_destination = argv[3];
	my_len = my_destination.size();
	for(int i = 0; i < my_len; ++i){
		if(isdigit(my_destination[i]) == FALSE){
			path_flag = 1;
		}
	}
	if(my_len==1){				inninsert(argv,2);return 0;}
	my_destination = my_destination + '/';

				my_len = my_destination.size();

				path.push_back("/");

				for(int i = 0; i < my_len; ++i){
					if(my_destination[i] != '/'){
						token = token + my_destination[i];
					}
					else{
						if((int)token.size() != 0){
							path.push_back(token);
						}
						token = "";
					}
				}
				for(int h=0;h<path.size();h++)
				{
					//cout<<"path["<<h<<"]"<<path[h]<<endl;
				}

				fd=open(argv[1],O_RDWR);
				//start from root -> 2.inode
			lseek(fd, BASE_OFFSET, SEEK_SET);
				read(fd, &super,sizeof(super));
				int BLOCK_SIZE=(1<<(10+super.s_log_block_size));

				//cout<<"SUPER: "<<super.s_free_blocks_count<<endl;


					int group_desc_offset;
					if(BLOCK_SIZE==1024)
					{
						group_desc_offset = BASE_OFFSET + BLOCK_SIZE ;

					}
					else
					{
						group_desc_offset = BLOCK_SIZE ;
					}

					lseek(fd,group_desc_offset,SEEK_SET);

					read(fd, &group_find_inode, sizeof(group_find_inode));//group desc
					//cout<<"fgi: "<<group_find_inode.bg_free_blocks_count<<endl;
					//std::cout << "/* message */" << '\n';
					inode_offset = 1;
					//std::cout << "/* message */" << '\n';
					int inode_offset_from_fd_find = (group_find_inode.bg_inode_table)*BLOCK_SIZE + (super.s_inode_size);
					//std::cout << "/* message */" << '\n';
					lseek(fd ,(group_find_inode.bg_inode_table)*BLOCK_SIZE + sizeof(inode_find),SEEK_SET);
					read(fd, &inode_find, sizeof(inode_find));
					//std::cout << inode_find.i_block[0]<<"/* message */" << '\n';
					struct ext2_dir_entry idir;
					unsigned int rdi;
					unsigned short rcl,trs;
					unsigned char ft,nl;
					int new_offset;
					char dname[255];
					trs=0;
					lseek(fd,inode_find.i_block[0]*BLOCK_SIZE+trs,SEEK_SET);
					read(fd,&rdi,sizeof(unsigned int));
					//std::cout << "/* message1 */" << '\n';
					//cout<<"first inode:"<<rdi;
					//std::cout << "/* message2 */" << '\n';

					int ii=0;int f1=0;int di=0;int pi=1;int f2=0;
					while(1)
					{
						trs=0;
						while(1)
						{
							lseek(fd,inode_find.i_block[ii]*BLOCK_SIZE+trs,SEEK_SET);
							read(fd,&rdi,sizeof(unsigned int));
							if(rdi==0){break;}
							read(fd,&rcl,sizeof(unsigned short));
							read(fd,&nl,sizeof(unsigned char));
							lseek(fd,sizeof(unsigned char),SEEK_CUR);
							read(fd,&dname,sizeof(char)*(int)nl);
							trs+=rcl;
							for(int k=0;k<nl;k++)
							{
								if(dname[k]!=path[pi][k]){//cout<<"directory "<<di<<" inode: "<<rdi<<"|"<<trs<<"|"<<rcl<<"|"<<dname<<endl;
								f2=1;break;}
							}

							if(f2==1){di++;f2=0;continue;}
							//cout<<"directory "<<di<<" inode: "<<rdi<<"|"<<trs<<"|"<<rcl<<"|"<<dname<<endl;
							int new_g = floor(rdi/super.s_inodes_per_group);
							//cout<<rdi%super.s_inodes_per_group<<"newg: "<<new_g<<endl;
							if(BLOCK_SIZE==1024)
							{
								new_offset=BASE_OFFSET+BLOCK_SIZE+sizeof(group_find_inode)*new_g;
							}
							else
							{
								new_offset = BLOCK_SIZE+sizeof(group_find_inode)*new_g;
							}
							lseek(fd,new_offset,SEEK_SET);
							read(fd, &group_find_inode, sizeof(group_find_inode));//group desc
							//cout<<"gc: "<<group_find_inode.bg_free_blocks_count<<endl;
							lseek(fd ,(group_find_inode.bg_inode_table)*BLOCK_SIZE + sizeof(inode_find)*(int)(rdi%super.s_inodes_per_group-1),SEEK_SET);
							ii=0;pi++;
							read(fd, &inode_find, sizeof(inode_find));
							//cout<<"found the inode: "<<inode_find.i_block[0]<<endl;

							if(pi==path.size()){//std::cout << "ulaaaaa" << '\n';
							f1=1;break;}
							di++;
							trs=0;
							if(rcl==1024-trs){break;}



						}
						if(f1==1){break;}
						ii++;
						if(ii==12){break;}
					}
					//cout<<"confirm: "<<rdi<<endl;
					inninsert(argv,rdi);

}

int main(int argc, char *argv[])
{
	string my_destination = argv[3];
	int my_len = my_destination.size();
	int path_flag=0;
	for(int i = 0; i < my_len; ++i){
		if(isdigit(my_destination[i]) == FALSE){
			path_flag = 1;
		}
	}

	if(path_flag == 0)
	{
		int gin=atoi(argv[3]);
		inninsert(argv,gin);
	}
	else
	{
		wwp(argv);
	}


		return 0;
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

      int my_bboff =  (group.bg_block_bitmap-1)*BLOCK_SIZE ;
      int n=ceil(super.s_blocks_count/super.s_blocks_per_group);

      int i=2n+3;
      char a;int bmf=0;
      while(i<BLOCK_SIZE)
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
      my_group_offset = BASE_OFFSET+(super.s_blocks_per_group*BLOCK_SIZE*group_number);
      //cout<<fd<<"!!!!!!!!!!!!!!"<<endl;
      lseek(fd,(2*n+3+data_block_index)*BLOCK_SIZE+my_group_offset,SEEK_SET);
      inode.i_block[aai]=(2*n+3+data_block_index)*BLOCK_SIZE+my_group_offset;
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
