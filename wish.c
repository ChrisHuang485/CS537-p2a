#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <ctype.h>

char *path[10];
int path_flag = 0;

int error_message()
{
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message)); 

}

char *trim(char *line)
{
    char *end, *rs, *es;
    int len;
    rs = line;
    end = line + strlen(line) -1;
    es = end;

    while(rs<=end && isspace(*rs))
        rs++;
    while(es>=rs && isspace(*es))
        es--;
    len = (es < rs) ? 0:(es -rs) + 1;
    rs[len] = '\0';
    return rs;
}

int split_line(char *line, char *tokens[])
{
    int position = 0;
    const char delim = ' ';
    strtok(line, "\n");
    while(1)
    {
        tokens[position] = strsep(&line, &delim);

        if(tokens[position] == NULL)
            break;

        position++;
    }
    return position;
}


int cd(char *token[])
{
    chdir(token[0]);

}

int _path(char *token[])
{
    char temp[50];
    int i = 0;
    int res = 0;
    while(1)
    {
        if(path[i] == NULL)
        {
            break;
        }
        else
        {
            strcpy(temp, path[i]);
            strcat(temp, "/");
            strcat(temp, token[0]);
            res = access(temp, X_OK);
            if(res == 0)
            {
                token[0] = (char *)malloc(strlen(temp)+1);
                strcpy(token[0], temp);
                return 0;
            }
        }
        i++;
    }
    if(res != 0)
    {
        error_message();
    }
    //return -1;
}

void launch(char *tokens[])
{
    char *temp_[50];
    temp_[0] = tokens[0];
    int rc = _path(temp_);
    if(rc == -1)
    {
        exit(1);
    }
    int pid = fork();
    if(pid == 0)
    {  
        int exec = execv(temp_[0], tokens);
        if (exec == -1)
            exit(1);
    }
    else if (pid < 0)
    {
        exit(1);
    } 
    else
    {
        wait(NULL);
    }
}


void new_path(char *tokens[])
{
    int i = 0;
    while(1)
    {
        if(tokens[i] == NULL)
        {
            path[i] = NULL;
            if(i == 0)
                path_flag = 1;
            break;
        }
        else
        {
            path[i] = (char *) malloc(strlen(tokens[i]) + 1);
            strcpy(path[i], tokens[i]);
        }
        i++;
    }
}

int print_tokens(char *tokens[])
{
    int i = 0;
    while(1)
    {
        if(tokens[i] == NULL)
        {
            printf("\n");
            break;
        }
        else
        {
            printf("%s ", tokens[i]);
        }
        i++;
    }
}

int check_char(char *line, char *chr)
{
    if(strstr(line,chr))
    {
        return 0;
    }
    return -1;
}

int str_num(char *line, char *chr)
{
    int count = 0;
    while(*line)
    {
        if(*line == *chr)
            count++;
        line++;
    }
    return count;
    
}

int re_direction(char *line)
{
    int i = 0;
    char *chr = ">";
    char *sp = " ";
    char *temp_line;
    char *temp_file;
    char *redirection_file;
    if(str_num(line,chr) != 1)
    {
        error_message();
        return -1;
    }
    strtok(line, "\n");
    temp_line = strsep(&line, chr);
    temp_file = strsep(&line, chr);
    line = trim(temp_line);
    redirection_file = trim(temp_file);
    if(*line == '\0' || *redirection_file == '\0' || check_char(redirection_file, sp) == 0)
    {
        error_message();
        return -1;
    }
    int fd = open(redirection_file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    dup2(fd, 1);
    dup2(fd, 2);
    close(fd);

}

int main (int argc, char **info)
{
    path[0] = "/bin";
    char *line = NULL;
    char *input_tokens[100];
    ssize_t size = 0;
    int input;
    FILE *file;

    if(argc == 1)
    {
        file = stdin;
    }
    else if(argc == 2)
    {
        file = fopen(info[1], "r");
        if(file == NULL)
        {
            error_message();
            exit(1);
        }
    }
    else
    {
        error_message();
        exit(1);
    }
    while(1)
    {
        if(argc == 1)
        {
            printf("wish> ");
            input = getline(&line, &size, stdin);
        }
        else
        {
            input = getline(&line, &size, file);
        }

        if (input == -1)
        {
            exit(0);
        }

        else if(input == 1)
        {
            continue;
        }

        else
        {
            int orginal_output = dup(1);
            int orginal_error = dup(2);
            int len;
            char *chr = ">";
            char *if_sign = "if";
            char *then_sign = "then";
            char *fi_sign = "fi";
            char *redirection_file;
            char *if_line = (char *)malloc(sizeof(char *));
            char *line_tokens[50];

            
            if(check_char(line, if_sign) == 0)
            {
                strcpy(if_line,  line);
                split_line(if_line,line_tokens);
                int i = 0;
                int j = 0;
                int k = 0;
                int if_num = 0;
                int int_position[10];
                int pn = 0;
                int then_num = 0;
                int fi_num = 0;
                while(1)
                {
                    if(line_tokens[i] == NULL)
                        break;
                    if(check_char(line_tokens[i],if_sign) == 0)
                    {
                        if(strlen(line_tokens[i]) != 2)
                        {
                            error_message();
                            break;
                        }
                        if_num++;
                        int_position[pn] = i;
                        pn++;
                    }
                   i++;
                }
               while(1)
                {
                    if(line_tokens[j] == NULL)
                        break;
                    if(check_char(line_tokens[j],then_sign) == 0)
                    {
                        if(strlen(line_tokens[j]) != 4)
                        {
                            error_message();
                            break;
                        }
                        then_num++;
                    }
                   j++;
                }
                
                while(1)
                {
                    if(line_tokens[k] == NULL)
                        break;
                    if(check_char(line_tokens[k],fi_sign) == 0)
                    {
                        if(strlen(line_tokens[k]) != 2)
                        {
                            error_message();
                            break;
                        }
                        fi_num++;
                    }
                   k++;
                }
                char *equal_out[if_num];
                int x = 0;
                int a = 1;
                int if_flag = 0;
                if(if_num == 1)
                {
                    while(1)
                    {
                        if(line_tokens[x]== NULL)
                            break;
                        if(strcmp(line_tokens[x], then_sign))
                        {
                            while(1)
                            {
                                if(strcmp(line_tokens[x],line_tokens[a]))
                                    break;
                                strcat(equal_out[0],&line_tokens[a]);
                                a++;
                            }
                        }
                        x++;
                    }
                
                equal_out[0] = trim(equal_out[0]);
//                printf("(%s)\n",equal_out[0]);
                
                }

                int pid = fork();
                if(pid == 0)
                {  
                    int exec = execv(line_tokens[1], equal_out[0]);
                    if (exec == -1)
                        exit(1);
                }
                else if (pid < 0)
                {
                    exit(1);
                } 
                else
                {
                    wait(NULL);
                }
//                if(if_num != then_num || if_num != fi_num || then_num != fi_num)
//                {
//                    error_message();
//                }



//                printf("(%d)\n",if_num);
//                printf("(%d)\n",then_num);
//                printf("(%d)\n",fi_num);
//                exit(0); // test
            }

            if(check_char(line, chr) == 0)
            {
                if(re_direction(line) != 0 )
                    continue;
                len = split_line(line, input_tokens);
            }
            else
            {
                line = trim(line);
                len = split_line(line, input_tokens);
            }

            if(strcmp(input_tokens[0], "exit") == 0)
            {
                if(len >= 2)
                {
                    error_message();
                }
                else
                {
                exit(0);
                }
            }

            else if(strcmp(input_tokens[0], "cd") == 0)
            {
                if(len == 2)
                {
                    cd(input_tokens+1);
                    continue;
                } 
                else
                {
                    error_message();
                }
            }
            else if(strcmp(input_tokens[0], "path") == 0)
            {
                new_path(input_tokens + 1);
//                print_tokens(path);
                continue;
            }
            else 
            {
                if(path_flag == 0)
                {
                    dup2(orginal_output,1);
                    dup2(orginal_error,2);
                    launch(input_tokens);
                }
                else
                {
                    error_message();
                    
                }
            }
        }
    }
}

