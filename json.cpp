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
        list(list&& s);
        void append(json elem);
        void prepend(json elem);
        list& operator=(const list& x);
        list& operator=(list&& x);

    private:

        friend struct json::list_iterator;
        friend struct json::const_list_iterator;
        friend json::list_iterator json::begin_list();
        friend json::const_list_iterator json::begin_list() const;

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
        dictionary(dictionary&& s);

        void append(std::pair<string, json> elem);
        void prepend(std::pair<string, json> elem);

        dictionary& operator=(const dictionary& x);
        dictionary& operator=(dictionary&& x);

        json const& operator[](std::string const& key) const;
        json& operator[](std::string const& key);

    private:

        friend struct json::dictionary_iterator;
        friend struct json::const_dictionary_iterator;
        friend json::dictionary_iterator json::begin_dictionary();
        friend json::const_dictionary_iterator json::begin_dictionary() const;

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
    int type_j; // utilizzo una variabile di tipo int per gestire la tipologia di json
    list list_j;
    dictionary dict_j;
    std::string string_j;
    double num_j;
    bool bool_j;
};

// List methods

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

list::list(list&& s) {
    *this = std::move(s);
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
        destroy(head);
        head = copy(x.head);
    }
    return *this;
}

list& list::operator=(list&& x) {
    if (this != &x) {
        destroy(head);
        head = x.head;
        tail = x.tail;
        x.head = x.tail = nullptr;
    }
    return *this;
}


// Dictionary methods

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

dictionary::dictionary(dictionary&& s){
    *this = std::move(s);
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
        destroy(head);
        head = copy(x.head);
    }
    return *this;
}

dictionary& dictionary::operator=(dictionary&& x) {
    if (this != &x) {
        destroy(head);
        head = x.head;
        tail = x.tail;

        x.head = x.tail = nullptr;
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
    std::pair<string, json> el;
    el.first = key;
    el.second = jsn;
    append(el);
    return tail->info.second;
}

// Json methods

json::json(){
    pimpl = new impl;
    pimpl->type_j = 0;
    pimpl->num_j = 0;
    pimpl->string_j = "";
}

json::json(json const& s) {
    pimpl = new impl;
    *this = s;  
}

json::json(json&& s) {
    pimpl = new impl;  
    *this = std::move(s);
}

json::~json() {
    delete pimpl;
}

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
        pimpl->type_j = s.pimpl->type_j;
        if (s.is_null()) {/*niente*/}
        else if (s.is_list()) pimpl->list_j = std::move(s.pimpl->list_j);
        else if (s.is_dictionary()) pimpl->dict_j = std::move(s.pimpl->dict_j);
        else if (s.is_string()) pimpl->string_j = std::move(s.pimpl->string_j);
        else if (s.is_number()) pimpl->num_j = std::move(s.pimpl->num_j);
        else if (s.is_number()) pimpl->bool_j = std::move(s.pimpl->bool_j);

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

/*  IMPLEMENTAZIONE ITERATORI  */

struct json::list_iterator {
    using value_type = json;
    using pointer = json*;
    using reference = json&;

    list_iterator(list::pcell ptr);

    reference operator*() const;
    pointer operator->() const;
    list_iterator& operator++();
    list_iterator operator++(int /* dummy */);

    bool operator==(list_iterator const& rhs) const;
    bool operator!=(list_iterator const& rhs) const;
    operator bool() const;

    private:
        list::pcell m_ptr;
};

json::list_iterator::list_iterator(list::pcell ptr) : m_ptr(ptr) {}

json& json::list_iterator::operator*() const {
    return m_ptr->info;
}

json* json::list_iterator::operator->() const {
    return &(m_ptr->info);
}

json::list_iterator& json::list_iterator::operator++() {
    m_ptr = m_ptr->next;
    return *this;
}

json::list_iterator json::list_iterator::operator++(int /* dummy */) {
    list_iterator temp = *this;
    ++(*this);
    return temp;
}

bool json::list_iterator::operator==(list_iterator const& rhs) const {
    return m_ptr == rhs.m_ptr;
}

bool json::list_iterator::operator!=(list_iterator const& rhs) const {
    return !(*this == rhs);
}

json::list_iterator::operator bool() const {
    return m_ptr != nullptr;
}

json::list_iterator json::begin_list() {
    if (!is_list()) {
        throw json_exception{"Invalid list"};
    }
    return list_iterator{pimpl->list_j.head};
}

json::list_iterator json::end_list() {
    if (!is_list()) {
        throw json_exception{"Invalid list"};
    }
    return list_iterator{nullptr};
}

/*  IMPLEMENTAZIONE CONST LIST ITERATOR  */

struct json::const_list_iterator {
    using value_type = const json;
    using pointer = const json*;
    using reference = const json&;

    const_list_iterator(list::pcell ptr);

    reference operator*() const;
    pointer operator->() const;
    const_list_iterator& operator++();
    const_list_iterator operator++(int /* dummy */);

    bool operator==(const_list_iterator const& rhs) const;
    bool operator!=(const_list_iterator const& rhs) const;
    operator bool() const;

    private:
        list::pcell m_ptr;
};

json::const_list_iterator::const_list_iterator(list::pcell ptr) : m_ptr(ptr) {}

json::const_list_iterator::reference json::const_list_iterator::operator*() const {
    return m_ptr->info;
}

json::const_list_iterator::pointer json::const_list_iterator::operator->() const {
    return &(m_ptr->info);
}

json::const_list_iterator& json::const_list_iterator::operator++() {
    m_ptr = m_ptr->next;
    return *this;
}

json::const_list_iterator json::const_list_iterator::operator++(int /* dummy */) {
    const_list_iterator temp = *this;
    ++(*this);
    return temp;
}

bool json::const_list_iterator::operator==(const_list_iterator const& rhs) const {
    return m_ptr == rhs.m_ptr;
}

bool json::const_list_iterator::operator!=(const_list_iterator const& rhs) const {
    return !(*this == rhs);
}

json::const_list_iterator::operator bool() const {
    return m_ptr != nullptr;
}

json::const_list_iterator json::begin_list() const {
    if (!is_list()) {
        throw json_exception{"Invalid list"};
    }
    return const_list_iterator{pimpl->list_j.head};
}

json::const_list_iterator json::end_list() const {
    if (!is_list()) {
        throw json_exception{"Invalid list"};
    }
    return const_list_iterator{nullptr};
}

struct json::dictionary_iterator {
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::pair<std::string, json>;
    using pointer = std::pair<std::string, json>*;
    using reference = std::pair<std::string, json>&;

    dictionary_iterator(dictionary::pcell ptr);

    reference operator*() const;
    pointer operator->() const;
    dictionary_iterator& operator++();
    dictionary_iterator operator++(int /* dummy */);

    bool operator==(dictionary_iterator const& rhs) const;
    bool operator!=(dictionary_iterator const& rhs) const;
    operator bool() const;

private:
    dictionary::pcell m_ptr;
};

json::dictionary_iterator::dictionary_iterator(dictionary::pcell ptr) : m_ptr(ptr) {}

json::dictionary_iterator::reference json::dictionary_iterator::operator*() const {
    return m_ptr->info;
}

json::dictionary_iterator::pointer json::dictionary_iterator::operator->() const {
    return &(m_ptr->info);
}

json::dictionary_iterator& json::dictionary_iterator::operator++() {
    m_ptr = m_ptr->next;
    return *this;
}

json::dictionary_iterator json::dictionary_iterator::operator++(int /* dummy */) {
    dictionary_iterator copy = *this;
    m_ptr = m_ptr->next;
    return copy;
}

bool json::dictionary_iterator::operator==(dictionary_iterator const& rhs) const {
    return m_ptr == rhs.m_ptr;
}

bool json::dictionary_iterator::operator!=(dictionary_iterator const& rhs) const {
    return !(*this == rhs);
}

json::dictionary_iterator::operator bool() const {
    return m_ptr != nullptr;
}

json::dictionary_iterator json::begin_dictionary() {
    if (!is_dictionary())
        throw json_exception{"Invalid dictionary"};
        
    return dictionary_iterator{pimpl->dict_j.head};
}

json::dictionary_iterator json::end_dictionary() {
    if (!is_dictionary())
        throw json_exception{"Invalid dictionary"};
        
    return dictionary_iterator{nullptr};
}

struct json::const_dictionary_iterator {
    using value_type = const std::pair<std::string, json>;
    using pointer = const std::pair<std::string, json>*;
    using reference = const std::pair<std::string, json>&;

    const_dictionary_iterator(dictionary::pcell ptr);

    reference operator*() const;
    pointer operator->() const;
    const_dictionary_iterator& operator++();
    const_dictionary_iterator operator++(int /* dummy */);

    bool operator==(const_dictionary_iterator const& rhs) const;
    bool operator!=(const_dictionary_iterator const& rhs) const;
    operator bool() const;

    private:
        dictionary::pcell m_ptr;
};

json::const_dictionary_iterator::const_dictionary_iterator(dictionary::pcell ptr) : m_ptr(ptr) {}

json::const_dictionary_iterator::reference json::const_dictionary_iterator::operator*() const {
    return m_ptr->info;
}

json::const_dictionary_iterator::pointer json::const_dictionary_iterator::operator->() const {
    return &(m_ptr->info);
}

json::const_dictionary_iterator& json::const_dictionary_iterator::operator++() {
    m_ptr = m_ptr->next;
    return *this;
}

json::const_dictionary_iterator json::const_dictionary_iterator::operator++(int /* dummy */) {
    const_dictionary_iterator temp = *this;
    ++(*this);
    return temp;
}

bool json::const_dictionary_iterator::operator==(const_dictionary_iterator const& rhs) const {
    return m_ptr == rhs.m_ptr;
}

bool json::const_dictionary_iterator::operator!=(const_dictionary_iterator const& rhs) const {
    return !(*this == rhs);
}

json::const_dictionary_iterator::operator bool() const {
    return m_ptr != nullptr;
}

json::const_dictionary_iterator json::begin_dictionary() const {
    if (!is_dictionary()) {
        throw json_exception{"Invalid dictionary"};
    }
    return const_dictionary_iterator{pimpl->dict_j.head};
}

json::const_dictionary_iterator json::end_dictionary() const {
    if (!is_dictionary()) {
        throw json_exception{"Invalid dictionary"};
    }
    return const_dictionary_iterator{nullptr};
}

std::ostream& operator<<(std::ostream& lhs, json const& rhs) {
    if (rhs.is_null())
        lhs << "null";
    else if (rhs.is_list()) {
        lhs << "[";
        auto it = rhs.begin_list();
        while (it) {
            lhs << *it;
            ++it;
            if (it)
                lhs << ", ";
        }
        lhs << "]";
    } else if (rhs.is_dictionary()) {
        lhs << "{";
        auto it = rhs.begin_dictionary();
        while (it) {
            lhs << "\"" << it->first << "\": " << it->second;
            ++it;
            if (it)
                lhs << ", ";
        }
        lhs << "}";
    } else if (rhs.is_string())
        lhs << "\"" << rhs.get_string() << "\"";
    else if (rhs.is_number())
        // lhs << std::fixed << std::setprecision(16) <<rhs.get_number();
        lhs << std::fixed <<rhs.get_number();
    else if (rhs.is_bool())
        lhs << (rhs.get_bool() ? "true" : "false");
    
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
    else if ((c == '-') || (c >= '0' and c <= '9')) {
        lhs.putback(c);
        parse_number(lhs, rhs);
        char next;
        lhs >> next;
        if (next > 0)    // it should be NULL
            throw json_exception{"Number not valid"};
        lhs.putback(c);
    } else if (stream_is_true(lhs))
        rhs.set_bool(true);
    else if (!stream_is_true(lhs))
        rhs.set_bool(false);
    else
        throw json_exception{"Stream not valid"};
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
                lhs.ignore(3); 
            } else if (c == 'f') {
                jsn.set_bool(false);
                lhs.ignore(4); 
            } else if (c == 'n') {
                jsn.set_null();
                lhs.ignore(3); 
            } else if ((c == '-') || (c >= '0' and c <= '9')){
                lhs.putback(c);
                parse_number(lhs, jsn);
            } else
                throw json_exception{"Invalid Json"};
            rhs.push_back(jsn);
            lhs >> c;
            if (c != ']' && c != ',')
                throw json_exception{"Expected ']' or ','."};
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
                std::string key;
                parse_string(lhs, key);
                lhs >> c;
                if (c != ':')
                    throw json_exception{"Invalid Json"};
                lhs >> c;

                json val;
                if (c == '[') {
                    parse_list(lhs, val);
                } else if (c == '{') {
                    parse_dict(lhs, val);
                } else if (c == '\"') {
                    parse_string(lhs, val);
                } else if (c == 't') {
                    val.set_bool(true);
                    lhs.ignore(3);  
                } else if (c == 'f') {
                    val.set_bool(false);
                    lhs.ignore(4); 
                } else if (c == 'n') {
                    val.set_null();
                    lhs.ignore(3);
                } else if ((c == '-') || (c >= '0' and c <= '9')) {
                    lhs.putback(c);
                    parse_number(lhs, val);
                } else
                    throw json_exception{"Invalid Json"};
                std::pair<std::string, json> p;
                p.first = key;
                p.second = val;
                rhs.insert(p);
                lhs >> c;
                if (c != '}' && c != ',')
                    throw json_exception{"Expected ']' or ','."};
                if (c == ',') {
                    lhs >> c; 
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
    
    return rhs;
}

json& parse_string(std::istream& lhs, json& rhs) {
    std::string str;
    char c;
    lhs.get(c);
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
    lhs.get(c);
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





