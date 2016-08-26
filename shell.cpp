#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<string.h>
#include<sys/stat.h>
#include<fcntl.h>
using namespace std;
char cmd[1000];
struct commandType{
	int n_part;
	char *part[20];
};
int out_d,in_d;
int npart=0;
int pip_n=0;
int back=0;
char *in_file,*out_file;
commandType command[40];
int findArgt(char * ar){
	if(strcmp(ar,"|")==0) return 1;
	else if(strcmp(ar,"&")==0) return 2;
	else if(strcmp(ar,">")==0) return 3;
	else if(strcmp(ar,">>")==0) return 4;
	else if(strcmp(ar,"<")==0) return 5;
	return 0;
}
void exx(int in, int out,commandType *cmd){
	pid_t pid;
	if((pid=fork())==0){
		if(in!=0){
			dup2(in,0);
			close(in);
		}
		if(out!=1){
			dup2(out,1);
			close(out);
		}
		if(execvp(cmd->part[0],(char * const *)cmd->part)<0){
				cout<<"Could not execute command\n";
				exit(1);
		}
	}
}
void fork_pipes(void){
	char *base=command[0].part[0];
	if(strcmp(base,"cd")==0){
		if(command[0].part[1]){
			if(chdir(command[0].part[1])!=0){
				cout<<"Wrong path\n";
			}
		}
		else{
			cout<<"Path name required\n";
		}
	}
	else if(strcmp(base,"mkdir")==0){
		if(mkdir(command[0].part[1],0700)!=0){
			cout<<"Directory exists or Invalid name\n";
		}
	}
	else if(strcmp(base,"rmdir")==0){
		if(rmdir(command[0].part[1])!=0){
			cout<<"Invalid name or directory doesn't exist or not permitted\n";
		}
	}
	else if(strcmp(base,"pwd")==0){
		char cwd[1000];
		getcwd(cwd, sizeof(cwd));
		cout<<cwd<<endl;
	}
	else if(strcmp(base,"exit")==0){
		exit(0);
	}
	else if(strcmp(base,"ls")==0){
		pid_t pid;
		if((pid=fork())==0){
			execlp("ls","ls",NULL);
			exit(0);
		}
		else{
			wait(0);
		}
	}
	else{
		pid_t pid;
		int i,in,fd[2],out,inpt;
		in = 0;
		for(i=0;i<=pip_n;i++){
			pipe(fd);
			exx(in,fd[1],command+i);
			close(fd[1]);
			in = fd[0];
		}
		if((pid=fork())==0){
			if(in!=0){
				dup2(in,0);
				close(in);
			}
			if(out_d==2){
				out = open(out_file, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
				dup2(out,1);
				close(out);
			}
			else if(out_d==4){
				out = open(out_file, O_RDWR | O_APPEND | O_CREAT, 0777);
				dup2(out,1);
				close(out);
			}
			if(in_d==2){
				inpt = open(in_file,O_RDONLY);
				dup2(inpt,0);
				close(inpt);
			}
			if(execvp(command[i].part[0],(char * const *)command[i].part)<0){
				cout<<"Could not execute command\n";
				exit(1);
			}
		}
		else{
			if(back==0){
				int status;
				while(wait(&status)!=pid);
			}
		}
	}
}
void executeCommand(char *cmd){
	for(int i=0;i<pip_n;i++){
		for(int j=0;j<command[i].n_part;j++){
			command[i].part[j]=NULL;
		}
	}
	pip_n=0;
	in_file=NULL;
	out_file=NULL;
	in_d=0;
	out_d=0;
	npart=0;
	back=0;
	if(cmd[0]=='\0') return;
	char *token;
	int argt;
	token = strtok(cmd," ");
	while(token!=NULL){
		argt = findArgt(token);
		if(out_d==0 && argt!=1 && argt!=2 && argt!=5){
			command[pip_n].part[npart]=token;
			npart++;
			command[pip_n].part[npart]=NULL;
		}
		token=strtok(NULL," ");
		if(out_d==1){
			out_file = token;
			out_d=2;
		}
		if(out_d==3){
			out_file=token;
			out_d=4;
		}
		if(in_d==1){
			in_file=token;
			in_d=2;
		}
		if(token){
			argt = findArgt(token);
			if(argt==1 || argt==2){
				command[pip_n].part[npart]=NULL;
				command[pip_n].n_part = npart;
				npart=0;
				pip_n++;
				if(argt==2) back=1;
			}
			else if(argt==3) out_d=1;
			else if(argt==4) out_d=3;
			else if(argt==5) in_d=1;
		}
	}
	fork_pipes();
}
int main(){
	while(1){
		char cwd[1000];
		getcwd(cwd, sizeof(cwd));
		cout<<cwd;
		cout<<" $ ";
		cin.getline(cmd,1000);
		executeCommand(cmd);
	}
	return 0;
}
