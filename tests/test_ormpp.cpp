#include "ormpp/dbng.hpp"
#include "ormpp/sqlite.hpp"

#include <cassert>
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

    assert(sqlite.connect("test.db", "test"));
    assert(sqlite.create_datatable<Student>(ormpp_auto_key{"id"}));
    assert(sqlite.insert<Student>({"zz", 'N', 99, "2202"}) == 1);
    assert(sqlite.insert<Student>({"zz", 'N', 999, "2202"}) == 1);
    assert(sqlite.delete_records_s<Student>("name='zz' and age=99"));
    assert(sqlite.update<Student>({ "lkh", 'N', 21, "2201" }, "name") == 1);

    for (auto& [name, sex, age, classroom, id] : sqlite.query_s<Student>())
    {
        std::cout << "name: " << name << std::endl;
        std::cout << "sex: " << sex << std::endl;
        std::cout << "age: " << age << std::endl;
        std::cout << "classroom: " << classroom << std::endl << std::endl;
    }

    assert(sqlite.disconnect());
    return std::filesystem::remove("test.db");
}
