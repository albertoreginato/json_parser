#include <iostream>
#include <string>
#include <limits>
#include <assert.h>
#include <fstream>

#include "json.hpp"

using std::string;

json& parse_list(std::istream& lhs, json& rhs);
json& parse_dict(std::istream& lhs, json& rhs);
json& parse_string(std::istream& lhs, json& rhs);
json& parse_number(std::istream& lhs, json& rhs);
bool parse_boolean(std::istream& lhs, json& rhs);
bool stream_is_true(std::istream& lhs);
void parse_string(std::istream& lhs, std::string& str);


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
    while (pc != nullptr) {
        if (pc->info.first == key)
            return pc->info.second;
        pc = pc->next;
    }
    throw json_exception{"key not found"};
}

json& dictionary::operator[](std::string const& key) {
    std::cout << "entrato dictionary::operator[]" << std::endl;
    if (key == "") {
        throw json_exception{"invalid key"};
    }
    pcell pc = head;
    while (pc != nullptr) {
        if (pc->info.first == key)
            return pc->info.second;
        pc = pc->next;
    }
    json jsn;
    std::cout << "fine while del dictionary::operator[] " << std::endl;
    std::pair<string, json> el;
    el.first = key;
    el.second = jsn;
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
    std::cout << "entrato operator[] const" << std::endl;
    if (!is_dictionary()) {
        throw json_exception{"It is not a dictionary"};
    }
    return pimpl->dict_j[key];
}
json& json::operator[](std::string const& key) {
    std::cout << "entrato operator[]" << std::endl;
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
    pimpl->list_j.prepend(x);
}
void json::push_back(json const& x) {
    if (!is_list())
        throw json_exception{"not a list"};
    pimpl->list_j.append(x);
}
void json::insert(std::pair<std::string, json> const& x) {
    if (!is_dictionary())
        throw json_exception{"not a dictionary"};
    pimpl->dict_j.append(x);
}

std::ostream& operator<<(std::ostream& lhs, json const& rhs) {
    return lhs;
}


std::istream& operator>>(std::istream& lhs, json& rhs) {
    char c = 0;
    lhs >> c;  
    if(c == '[') {
        parse_list(lhs, rhs);
    } else if (c == '{') {
        parse_dict(lhs, rhs);
    } else if (c == '\"')
        parse_string(lhs, rhs);
    else if (c >= '0' and c <= '9') {
        lhs.putback(c);
        parse_number(lhs, rhs);
    } else if (lhs.rdbuf()->in_avail() == 0)  // se lo stream è vuoto !
        throw json_exception{"empty stream"};
    else if (stream_is_true(lhs))
        rhs.set_bool(true);
    else if (!stream_is_true(lhs))
        rhs.set_bool(false);
    else
        throw json_exception{"stream not valid"};
    return lhs;
}

json& parse_list(std::istream& lhs, json& rhs) {
    rhs.set_list();
    char c = 0;
    lhs >> c;

    while (c != ']') {
        if (c != ',') {
            json jsn;
            if (c == '[') {
                parse_list(lhs, jsn);
            } else if (c == '{') {
                parse_dict(lhs, jsn);
            } else if (c == '\"') {
                parse_string(lhs, jsn);
            } else if (c == 't') {
                jsn.set_bool(true);
                lhs.ignore(3);  // Ignora il resto di "true"
            } else if (c == 'f') {
                jsn.set_bool(false);
                lhs.ignore(4);  // Ignora il resto di "false"
            } else if (c == 'n') {
                jsn.set_null();
                lhs.ignore(3);  // Ignora il resto di "null"
            } else if (c >= '0' and c <= '9'){
                lhs.putback(c);
                parse_number(lhs, jsn);
            } else
                throw json_exception{"Invalid Json"};
            rhs.push_back(jsn);
            lhs >> c;
            if (c != ']' && c != ',')
                throw json_exception{"Invalid Json"};
            if (c == ',') {
                lhs >> c;  // Leggi il carattere successivo dopo la virgola
                if (c == ']')
                    throw json_exception{"Invalid Json"};
            }
        } else
            throw json_exception{"Invalid Json"};
    }
    return rhs;
}

json& parse_dict(std::istream& lhs, json& rhs) {
    rhs.set_dictionary();
    char c = 0;
    lhs >> c;
    while (c != '}') {
        if (c != ',') {
            if (c == '\"') {
                // Parsing della chiave utilizzando il nuovo metodo parse_string
                std::string key;
                parse_string(lhs, key);
                lhs >> c; // Salta il separatore dei due punti
                if (c != ':')
                    throw json_exception{"Invalid Json: expected colon after key"};
                lhs >> c; // Leggi il primo carattere del valore
                std::cout << "c: " << c << std::endl;

                // Parsing del valore
                json val;
                if (c == '[') {
                    parse_list(lhs, val);
                } else if (c == '{') {
                    parse_dict(lhs, val);
                } else if (c == '\"') {
                    parse_string(lhs, val);
                } else if (c == 't') {
                    val.set_bool(true);
                    lhs.ignore(3);  // Ignora il resto di "true"
                } else if (c == 'f') {
                    val.set_bool(false);
                    lhs.ignore(4);  // Ignora il resto di "false"
                } else if (c == 'n') {
                    val.set_null();
                    lhs.ignore(3);  // Ignora il resto di "null"
                } else if (c >= '0' and c <= '9') {
                    lhs.putback(c);
                    parse_number(lhs, val);
                } else
                    throw json_exception{"Invalid Json"};
                rhs[key] = val;
                lhs >> c;
                std::cout << "c: " << c << std::endl;
                if (c != '}' && c != ',')
                    throw json_exception{"Invalid Json"};
                if (c == ',') {
                    lhs >> c;  // Leggi il carattere successivo dopo la virgola
                    if (c == '}')
                        throw json_exception{"Invalid Json"};
                }
            } else
                throw json_exception{"Invalid Json"};
        } else
            throw json_exception{"Invalid Json"};
    }
    return rhs;
}

json& parse_number(std::istream& lhs, json& rhs) {
    double num;
    lhs >> num;
    rhs.set_number(num);
    std::cout << "num: " << num << std::endl;
    return rhs;
}

json& parse_string(std::istream& lhs, json& rhs) {
    std::string str;
    char c;
    lhs >> c;
    char prec = ' ';
    while(c != '\"' || (prec == '\\' && c == '\"')) {
        str += c;
        if (prec == c && c == '\\')
            prec = ' ';
        else    
            prec = c;
        lhs.get(c);
    }
    rhs.set_string(str);
    return rhs;
}



bool stream_is_true(std::istream& lhs) {
    std::string str;
    lhs >> str;
    if (str == "true")
        return true;
    return false;
}

void parse_string(std::istream& lhs, std::string& str){
    char c;
    lhs >> c;
    char prec = ' ';
    while(c != '\"' || (prec == '\\' && c == '\"')) {
        str += c;
        if (prec == c && c == '\\')
            prec = ' ';
        else    
            prec = c;
        lhs.get(c);
    }
}





