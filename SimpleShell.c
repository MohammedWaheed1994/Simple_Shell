#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

typedef enum
{
    Exit=0,
    No_Input,
    Service,
    Set,
    Export   
}UserCheck;


/*Structure for env vars*/
typedef struct{
char name[20] ;
char value[20]
}env_var;

/*Number of stored env variables*/
int number_env = 0;

/*List of stored  env variables*/
env_var Stored_env[20];

/*User input serialized*/
/*TODO: Revisit This array implementation and provide checks over size*/
char User_Input[200] = {0};
char *Par[10] = {NULL} ;
int NumberOfChar;
char User_InputSerialized[10][20] = {0};
char error = 0;

/*Check limitation of user input*/
void CheckWordAndChar(int charno, int wordno)
{
	if( (charno > 19U) || (wordno > 9U)) 
	{

		error = 1;
		write(1,"Data is too long!!!\n",20); 
	}

}

/*User input to be serialized*/
void SerializeUserInput(void)
{
	int iterator  = 0;
	int wordNo = 0;
	int CharNo = 0;
	int SpaceFlag = 0;
/*Clear buffer*/
	Par[0] = Par[1] = Par[2] = Par[3] = Par[4] = Par[5] = Par[6] = Par[7] = Par[8] = Par[9] =  NULL;

	for(iterator  = 0;iterator < NumberOfChar ; iterator++)
	{
		if(User_Input[iterator] == ' ')
		{
			if(SpaceFlag == 0)
			{
				User_InputSerialized[wordNo][CharNo] = 0;
				Par[wordNo] = User_InputSerialized[wordNo];
				wordNo++;
		       		CharNo = 0;
				SpaceFlag = 1;
				CheckWordAndChar(CharNo,wordNo);
			
				if(error == 1)break; 
			}	
		}
	
		else if(User_Input[iterator] == '\n')
		{
			if(iterator == 0)
			{
				Par[0] = NULL;
			}

			else if( SpaceFlag == 0)
			{
				User_InputSerialized[wordNo][CharNo] = 0;
				Par[wordNo] = User_InputSerialized[wordNo];

			}
			else  Par[wordNo] = NULL;
			

			wordNo++;
			Par[wordNo] = NULL;
			CheckWordAndChar(CharNo,wordNo);
			 if(error == 1)break;
			break;
		}

		else
		{
			User_InputSerialized[wordNo][CharNo] = User_Input[iterator];
			CharNo++;
			CheckWordAndChar(CharNo,wordNo);
			 if(error == 1)break;
			SpaceFlag = 0;
		}
	}

}


/*Check duplicate of the same env variable*/
int CheckDuplicate(int *env_number_output,char string[])
{
	int iterator = 0;
	int returnvalue = 0;

	for(iterator = 0;iterator < number_env; iterator++)
	{
		if(!strcmp(string, Stored_env[iterator].name))
		{	
			*env_number_output = iterator; 
			returnvalue = 1;
			break;
		}
	}
return returnvalue;

}

/*Function to store new variables*/
void VarStore()
{
	int iterator = 0;
	int error = 0;
	int StartOfValue = 0;
	int checkvalue = 0;
	int duplicatenumber = 0;
	int number = number_env;

	/*TODO: Enhance the check that only one '='*/
	/*Check if there is more space for new vars*/
	if(number_env < 20)
	{
		int ValueSetFlag = 0;
		for(iterator = 0; iterator < strlen(Par[0])  ;iterator++)
		{
			if( (ValueSetFlag == 0) && (Par[0][iterator] != '='))
			{
				Stored_env[number].name[iterator] = Par[0][iterator];

			}
			else if((Par[0][iterator] == '=') && (iterator != 0))
			{
				Stored_env[number].name[iterator] = 0;
				StartOfValue = iterator+1;
				ValueSetFlag = 1;
				checkvalue = CheckDuplicate(&duplicatenumber,Stored_env[number_env].name );
				
        		if(checkvalue == 1)
        		{
                		number = duplicatenumber;
        		}

			}
			else if ((Par[0][iterator] == '=') && (iterator == 0))
			{
				error = 1;
			}

			else if(ValueSetFlag == 1) 
			{
				
		       		if(!((Par[0][iterator] >=48 ) && (Par[0][iterator] <= 57)))
				{
					error = 1;
					break;	
				}
				else
				{
					Stored_env[number].value[iterator-StartOfValue] = Par[0][iterator];

				}	
			}
		}	

	}
	
	if(error == 0)
	{
		Stored_env[number].value[iterator-StartOfValue] = 0;
		if(checkvalue == 0)
		{
			number_env++;
		}
	}
	else
       	{
		write(1,"Incorrect user input\n",21);
	}
}

/*Using system calls to interface with standard input & output*/
UserCheck GUI_GetUserInput(void)
{
	UserCheck RetVal = No_Input;
	/*Write User interface */
	write(1,"Welcome to simple shell >",26 );
	
	/*TODO: for debugging: To be removed*/
	NumberOfChar = read(0,User_Input,200);
	SerializeUserInput();
	
	if(error == 1U)
	{	
			RetVal = Exit;
		}
		else
		{
		if(NumberOfChar  == 1)
		{
			RetVal = No_Input;
		}
	
		else if(NumberOfChar > 0) 
		{
			if(!strcmp(User_InputSerialized[0] ,"Exit"))
			{
				RetVal = Exit;
			}
			else if(!strcmp(User_InputSerialized[0] ,"set"))
			{
				RetVal = Set;
			}
			else if(!strcmp(User_InputSerialized[0] ,"export"))
			{
				RetVal = Export;
			}
	
			else if(strchr(User_InputSerialized[0] ,'=') != NULL)
			{
				VarStore();
			}	
			else RetVal = Service;

		}
	}
	return 	RetVal;
}

/*Set service hanlder*/
void SetHandler(void)
{
	int iterator = 0;

	for(iterator = 0; iterator < number_env; iterator++)
	{
		write(1,Stored_env[iterator].name,strlen(Stored_env[iterator].name));

		write(1,"=",1);

		write(1,Stored_env[iterator].value,strlen(Stored_env[iterator].value));

		write(1,"\n",1);
	}

}

/*Export handler*/
void ExportHandler(void)
{
	
	int iterator = 0;

	for(iterator = 0;iterator < number_env;iterator++)
	{
		if(setenv(Stored_env[iterator].name , Stored_env[iterator].value,1) == 0 );
		else 
		{	
			write(1,"export failed!\n",15);
			break;	
		}
		

	}

}

int main(int argc, char *argv[])
{
int Pid;
int status;
UserCheck UserRet = No_Input;	

while(1)
	{
		UserRet = GUI_GetUserInput();
		
		if(UserRet == Exit)
		{
			write(1,"Shell is terminating\n",22);
			break;
		}	

		else if(UserRet == Service)
		{
			Pid = fork();

			if(Pid == 0)
			{
				/*Child Process*/
				execvp(Par[0],Par);			
				
				/*TODO: Check types of errors*/
				write(1,"Process cannot be created\n",27);	
			}
			else if(Pid > 0)
			{
				/*Parent Process*/	
				wait(&status);
			}
	
			else  write(1,"Child Process cannot be created\n",35); 
		}
		else if(UserRet == Set)
		{
			SetHandler();
		}

		else if(UserRet == Export)
		{
			ExportHandler();
		}

		else {}
	
	}
	return 0;
}
