#include <iostream>
#include <string>
#include <limits>
#include <assert.h>
#include <fstream>

#include "json.hpp"

using std::string;



class list {
    public:
    
        list();
        ~list();
        list(const list& s);

        void append(json elem);
        void prepend(json elem);
        

        list& operator=(const list& x);

    private:
        struct cell {
            json info;
            cell* next;
        };
        typedef cell* pcell;
        pcell head;
        pcell tail;
        void destroy(pcell pc);
        pcell copy(pcell pc);
};

class dictionary {
    public: 
        dictionary();
        ~dictionary();
        dictionary(const dictionary& s);

        void append(std::pair<string, json> elem);
        void prepend(std::pair<string, json> elem);

        dictionary& operator=(const dictionary& x);

        json const& operator[](std::string const& key) const;
        json& operator[](std::string const& key);

    private:
        struct cell {
            std::pair<std::string, json> info;
            cell* next;
        };
        typedef cell* pcell;
        pcell head;
        pcell tail;
        void destroy(pcell pc);
        pcell copy(pcell pc);
};



struct json::impl {
    int type_j;
    list list_j;
    dictionary dict_j;
    std::string string_j;
    double num_j;
    bool bool_j;
};



// LIST METHODS             ************************************************************************

list::list() {
    head = nullptr;
    tail = nullptr;
}

list::~list() {
    destroy(head);
}

void list::destroy(cell* pc) {
    if (pc) {
        destroy(pc->next);
        delete pc;
    }
}

list::list(const list& s) {
    head = nullptr;
    tail = nullptr;
    head = copy(s.head);
}

list::pcell list::copy(pcell s) {
    if (!s)
        return nullptr;
    else {
        pcell dest = new cell;
        dest->info = s->info;
        dest->next = copy(s->next);
        return dest;
    }
}

void list::prepend(json el) {
    pcell new_one = new cell;
    new_one->info = el;
    new_one->next = head;
    head = new_one;
    if (head->next == nullptr)
        tail = new_one;
}

void list::append(json el) {
    pcell new_one = new cell;
    new_one->info = el;
    new_one->next = nullptr;
    if (tail!=nullptr)
        tail->next = new_one;
    else 
        head = new_one;
    tail = new_one;
}

list& list::operator=(const list& x) {
    if (this != &x) {
        // destroy my list
        destroy(head);
        head = copy(x.head);
    }
    return *this;
}


// DICTIONARY METHODS ************************************************************************

dictionary::dictionary() {
    head = nullptr;
    tail = nullptr;
}

dictionary::~dictionary() {
    destroy(head);
}

void dictionary::destroy(cell* pc) {
    if (pc) {
        destroy(pc->next);
        delete pc;
    }
}

dictionary::dictionary(const dictionary& s) {
    head = nullptr;
    tail = nullptr;
    head = copy(s.head);
}

dictionary::pcell dictionary::copy(pcell s) {
    if (!s)
        return nullptr;
    else {
        pcell dest = new cell;
        dest->info.first = s->info.first;
        dest->info.second = s->info.second;
        dest->next = copy(s->next);
        return dest;
    }
}

void dictionary::prepend(std::pair<string, json> el) {
    pcell new_one = new cell;
    new_one->info.first = el.first;
    new_one->info.second = el.second;
    new_one->next = head;
    head = new_one;
    if (head->next == nullptr)
        tail = new_one;
}

void dictionary::append(std::pair<string, json> el) {
    pcell new_one = new cell;
    new_one->info.first = el.first;
    new_one->info.second = el.second;
    new_one->next = nullptr;
    if (tail!=nullptr)
        tail->next = new_one;
    else 
        head = new_one;
    tail = new_one;
}

dictionary& dictionary::operator=(const dictionary& x) {
    if (this != &x) {
        // destroy my dictionary
        destroy(head);
        head = copy(x.head);
    }
    return *this;
}

json const& dictionary::operator[](std::string const& key) const {
    if (key == "") {
        throw json_exception{"invalid key"};
    }
    pcell pc = head;
    while (pc->next != nullptr) {
        if (pc->info.first == key)
            return pc->info.second;
        pc = pc->next;
    }
    throw json_exception{"key not found"};
}

json& dictionary::operator[](std::string const& key) {
    if (key == "") {
        throw json_exception{"invalid key"};
    }
    pcell pc = head;
    while (pc->next != nullptr) {
        if (pc->info.first == key)
            return pc->info.second;
        pc = pc->next;
    }
    std::pair<string, json> el;
    el.first = key;
    el.second = json();
    append(el);
    return tail->info.second;
}



// JSON METHODS ************************************************************************

json::json(){
    pimpl = new impl;
    pimpl->type_j = 0;
    pimpl->num_j = 0;
    pimpl->string_j = "";
    // list e dictionary hanno i loro default constructor
}

json::json(json const& s) {
    pimpl = new impl;
    *this = s;  // operatore =
}
json::json(json&& s) {

}
json::~json() {
    // std::cout << "distruttore json chiamato" << std::endl;
    delete pimpl;
}


// OVERLOADING OPERATORI

json& json::operator=(json const& s){
    if(this != &s){
        delete pimpl;
        pimpl = new impl;
        pimpl->type_j = s.pimpl->type_j;
        if (is_list())
            pimpl->list_j = s.pimpl->list_j;
        else if (is_dictionary())
            pimpl->dict_j = s.pimpl->dict_j;
        else if (is_string())
            pimpl->string_j = s.pimpl->string_j;
        else if (is_number())
            pimpl->num_j = s.pimpl->num_j;
        else if (is_bool())
            pimpl->bool_j = s.pimpl->bool_j;
        
    }
    return *this;
}

json& json::operator=(json&& s) {
    if(this != &s){
        delete pimpl;
        pimpl = new impl;
        pimpl = s.pimpl;
        s.pimpl = nullptr;
    }
    return *this;
}

bool json::is_list() const{
    return pimpl->type_j == 1;
}
bool json::is_dictionary() const{
    return pimpl->type_j == 2;
}
bool json::is_string() const{
    return pimpl->type_j == 3;
}
bool json::is_number() const{
    return pimpl->type_j == 4;
}
bool json::is_bool() const{
    return pimpl->type_j == 5;
}
bool json::is_null() const{
    return pimpl->type_j == 0;
}



json const& json::operator[](std::string const& key) const {
    if (!is_dictionary()) {
        throw json_exception{"It is not a dictionary"};
    }
    return pimpl->dict_j[key];
}
json& json::operator[](std::string const& key) {
    if (!is_dictionary()) {
        throw json_exception{"It is not a dictionary"};
    }
    return pimpl->dict_j[key];
}








double& json::get_number() {
    if (!is_number())
        throw json_exception{"not a number"};
    return pimpl->num_j;
}
double const& json::get_number() const {
    if (!is_number())
        throw json_exception{"not a number"};
    return pimpl->num_j;
}
bool& json::get_bool() {
    if (!is_bool())
        throw json_exception{"not a boolean"};
    return pimpl->bool_j;
}
bool const& json::get_bool() const {
    if (!is_bool())
        throw json_exception{"not a boolean"};
    return pimpl->bool_j;
}
std::string& json::get_string() {
    if (!is_string())
        throw json_exception{"not a string"};
    return pimpl->string_j;
}
std::string const& json::get_string() const {
    if (!is_string())
        throw json_exception{"not a string"};
    return pimpl->string_j;
}

/* METODI PER SETTARE JSON */

void json::set_string(std::string const& x) {
    delete pimpl;
    pimpl = new impl;
    pimpl->string_j = x;
    pimpl->type_j = 3;
}
void json::set_bool(bool x) {
    delete pimpl;
    pimpl = new impl;
    pimpl->bool_j = x;
    pimpl->type_j = 5;
}
void json::set_number(double x) {
    delete pimpl;
    pimpl = new impl;
    pimpl->num_j = x;
    pimpl->type_j = 4;
}
void json::set_null() {
    delete pimpl;
    pimpl = new impl;
    pimpl->type_j = 0;
}
void json::set_list() {
    delete pimpl;
    pimpl = new impl;
    pimpl->type_j = 1;
}
void json::set_dictionary() {
    delete pimpl;
    pimpl = new impl;
    pimpl->type_j = 2;
}
void json::push_front(json const& x) {
    if (!is_list())
        throw json_exception{"not a list"};
    pimpl->list_j.append(x);
}
void json::push_back(json const& x) {
    if (!is_list())
        throw json_exception{"not a list"};
    pimpl->list_j.prepend(x);
}
void json::insert(std::pair<std::string, json> const& x) {
    if (!is_dictionary())
        throw json_exception{"not a dictionary"};
    pimpl->dict_j.prepend(x);
}


