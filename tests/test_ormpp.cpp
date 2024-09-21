#include "ormpp/dbng.hpp"
#include "ormpp/sqlite.hpp"

#include <filesystem>

struct Student
{
    std::string name;
    char sex;
    int age;
    std::string classroom;
    int id;
};
REGISTER_AUTO_KEY(Student, id)
REFLECTION(Student, name, sex, age, classroom, id)

int main()
{
    ormpp::dbng<ormpp::sqlite> sqlite;
    sqlite.connect("test.db");

    {
        sqlite.create_datatable<Student>(ormpp_auto_key{"id"});
        sqlite.insert<Student>({"zz", 'N', 99, "2202"});
        sqlite.insert<Student>({"zz", 'N', 999, "2202"});

        if(sqlite.delete_records_s<Student>("name='zz' and age=99"))
        {
            std::cout << "OK" << std::endl;
        }

        for (auto& [name, sex, age, classroom, id] : sqlite.query_s<Student>())
        {
            std::cout << "name: " << name << std::endl;
            std::cout << "sex: " << sex << std::endl;
            std::cout << "age: " << age << std::endl;
            std::cout << "classroom: " << classroom << std::endl << std::endl;
        }
    }

    sqlite.disconnect();

    return 0;
}
