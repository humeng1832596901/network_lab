/*************************************************************************
	> File Name: db.h
	> Author: 
	> Mail: 
	> Created Time: Mon 02 Nov 2020 08:38:51 AM CST
 ************************************************************************/
#include<stdio.h>
#include<sqlite3.h>

int main(){
    const char* select_query = "select * from users";
    int ret = 0;
    sqlite3 *db = 0;
    char* s;
    
    //打开数据库，如果不存在则创建数据库db
    ret = sqlite3_open("./mydb",&db);
    if(ret != SQLITE_OK){
        printf("can't open db\n");
        return 1;
    }
    printf("open successfully\n");

    //创建表
    ret = sqlite3_exec(db,"create table if not exists users(user char[20],pwd char[20],sex char[10])",0,0,&s);
    if(ret != SQLITE_OK){
        sqlite3_close(db);
        printf("create error\n");
        return 1;
    }
    printf("create successfully\n");
}

