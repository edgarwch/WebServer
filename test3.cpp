#include "WebServer/MemoryPool.h"
#include <memory>
#include <iostream>
#include <string>

using namespace std;

class Person{
    public:
        int id_;
        string name_;
        Person(int id, string name):id_(id), name_(name){ 
            printf("Person constructor\n");
        }
        ~Person(){ 
            printf("Person destructor\n"); 
        }
};


void test(){
    printf("Creating a person...\n");

    shared_ptr<Person> p(NewElement<Person>(112, "zhangsan"), DeleteElement<Person>);
    printf("sizeof(name_) = %d\n", sizeof(p->name_));
    printf("sizeof(id_) = %d\n", sizeof(p->id_));
}
void test2(){
    printf("Creating a person...\n");

    shared_ptr<Person> p(NewElement<Person>(112, "zhang123123san"), DeleteElement<Person>);
    printf("sizeof(name_) = %d\n", sizeof(p->name_));
    printf("sizeof(id_) = %d\n", sizeof(p->id_));
}
int main(){
    Init_MemoryPool();
    test();
    test2();
    return 0;
}