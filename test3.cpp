#include "WebServer/MemoryPool.h"
#include <memory>
#include <iostream>
#include <string>

using namespace std;

class Person{
    public:
        int id;
        string name;
        Person(int id, string name):id(id), name(name){ printf("Person constructor\n"); }
        ~Person(){ printf("Person destructor\n"); }
};


void test(){
    printf("Creating a person...\n");\
    shared_ptr<Person> p(NewElement<Person>(112, "zhangsan"), DeleteElement<Person>);
    printf("sizeof(name_) = %d\n", sizeof(p->name));
    printf("sizeof(id_) = %d\n", sizeof(p->id));
}

int main(){
    test();
    return 0;
}