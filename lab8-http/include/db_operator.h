/*************************************************************************
	> File Name: db_operator.h
	> Author: 
	> Mail: 
	> Created Time: Mon 02 Nov 2020 08:52:35 AM CST
 ************************************************************************/

#ifndef _DB_OPERATOR_H
#define _DB_OPERATOR_H
#include<stdio.h>
#include<string.h>
#include<sqlite3.h>

int s_search(char* user, char* pwd, char* sex);

sqlite3* opendb(){
    sqlite3* db = 0;
    int ret = sqlite3_open("./db/mydb",&db);
    if(ret != SQLITE_OK){
        return NULL;
    }
    //printf("open successfully\n");
    return db;
}

//插入的时候要判断，如果信息重合则插入失败
int s_insert(char* user, char* pwd, char* sex){
    //printf("xixi\n");
    if(s_search(user,pwd,sex)==0){
        printf("insert error, existed!\n");
        return 1;
    }
    sqlite3* db = opendb();
    if(db==NULL){
        printf("open error in insert function\n");
        return 1;
    }
    char* s;
    char info[100];
    sprintf(info,"insert into users values('%s','%s','%s')",user,pwd,sex);
    int ret = sqlite3_exec(db,info,0,0,&s);
    if(ret != SQLITE_OK)
        return 1;
    else{
        printf("insert successfully\n");
        return 0;
    }
}

int s_delete(){

}

int s_update(){

}

int s_search(char* user, char* pwd, char* sex){
    sqlite3* db = opendb();
    if(db == NULL)
        return 1;

    char* s;  //存储错误信息
    //const char* select_query = "select * from users";
    char order[100];
    sprintf(order,"select * from users where (user=\"%s\" and pwd=\"%s\" and sex=\"%s\")",user,pwd,sex);
    int nrow, ncolumn; //返回查询到的行数和列数
    char** db_result;
    //printf("%s\n",order);
    int ret = sqlite3_get_table(db,order,&db_result,&nrow,&ncolumn,&s);
    if(ret != SQLITE_OK){
        sqlite3_close(db);
        return 1;
    }
    if(nrow!=0 && ncolumn!=0){
        //printf("%d %d\n",nrow,ncolumn);
        sqlite3_close(db);
        return 0;
    }else{
        sqlite3_close(db);
        return 1;
    }
    //printf("nrow=%d ncolumn=%d\n",nrow,ncolumn);
    //if(nrow!=0 && ncolumn!=0){
        //printf("%s %s %s\n",user,pwd,sex);
        //printf("%s %s %s\n",db_result[3],db_result[4],db_result[5]);
        
   //     if(strcmp(db_result[3],user)==0 && strcmp(db_result[4],pwd)==0 && strcmp(db_result[5],sex)==0){
   //         sqlite3_close(db);
   //         return 0;
   //     }else{
            //printf("adjust error\n");
   //         return 1;
   //     }
        
   // }else{
        //printf("not found\n");
   //     sqlite3_close(db);
   //     return 1;
   // }
}

#endif
