#include <iostream>
#include <vector>
#include <map>
#include <variant>
#include <complex>
#include <iomanip>
#include <string>
#include <initializer_list>
#include <utility>
#include <stdexcept>
#include <iterator>
#include <algorithm>

using namespace std;

struct Var;

using VarVariant = variant<
    int,
    double,
    complex<double>,
    vector<Var>,
    map<string, Var>,
    string
>;

struct Var {
private:
    VarVariant data;

public:
    Var() = default;
    Var(int v) : data(v) {}
    Var(double v) : data(v) {}
    Var(complex<double> v) : data(v) {}
    Var(const vector<Var>& v) : data(v) {}
    Var(const map<string, Var>& v) : data(v) {}
    Var(const string& v) : data(v) {}
    Var(const char* v) : data(string(v)) {}
    Var(const initializer_list<Var>& v) : data(vector<Var>(v)) {}

public:
    Var& operator=(int v) { data = v; return *this; }
    Var& operator=(double v) { data = v; return *this; }
    Var& operator=(complex<double> v) { data = v; return *this; }
    Var& operator=(const vector<Var>& v) { data = v; return *this; }
    Var& operator=(const map<string, Var>& v) { data = v; return *this; }
    Var& operator=(const string& v) { data = v; return *this; }
    Var& operator=(const char* v) { data = string(v); return *this; }
    Var& operator=(const initializer_list<Var>& v) {
        data = vector<Var>(v);
        return *this;
    }

    string type() const {
        if (is_int())      return "int";
        if (is_double())   return "double";
        if (is_complex())  return "complex";
        if (is_string())   return "string";
        if (is_list())     return "list";
        if (is_dict())     return "dict";
        return "unknown";
    }

    bool is_int() const         { return holds_alternative<int>(data); }
    bool is_double() const      { return holds_alternative<double>(data); }
    bool is_string() const      { return holds_alternative<string>(data); }
    bool is_complex() const     { return holds_alternative<complex<double>>(data); }
    bool is_list() const        { return holds_alternative<vector<Var>>(data); }
    bool is_dict() const        { return holds_alternative<map<string, Var>>(data); }

public:
    size_t size() const {
        if (is_list())  return get<vector<Var>>(data).size();
        if (is_dict())  return get<map<string, Var>>(data).size();
        return 1;
    }

    void append(const Var& val) {
        if (is_list())
            get<vector<Var>>(data).push_back(val);
    }

    void set(const string& key, const Var& val) {
        if (!is_dict())
            data = map<string, Var>{};
        get<map<string, Var>>(data)[key] = val;
    }

    Var& operator[](int pos) {
        if (is_list())
            return get<vector<Var>>(data)[pos];
        return *this;
    }

    const Var& operator[](int pos) const {
        if (is_list())
            return get<vector<Var>>(data)[pos];
        return *this;
    }

    Var& operator[](const string& key) {
        if (!is_dict())
            data = map<string, Var>{};
        return get<map<string, Var>>(data)[key];
    }

    const Var& operator[](const string& key) const {
        if (is_dict())
            return get<map<string, Var>>(data).at(key);
        throw runtime_error("not a map");
    }

public:
    struct iterator {
        using value_type = Var;
        using reference = const Var&;
        using pointer = const Var*;
        using difference_type = ptrdiff_t;
        using iterator_category = forward_iterator_tag;

        variant<
            vector<Var>::const_iterator,
            map<string, Var>::const_iterator,
            const Var*
        > it;

        iterator(vector<Var>::const_iterator i) : it(i) {}
        iterator(map<string, Var>::const_iterator i) : it(i) {}
        iterator(const Var* v) : it(v) {}

        reference operator*() const {
            if (holds_alternative<vector<Var>::const_iterator>(it))
                return *get<vector<Var>::const_iterator>(it);
            if (holds_alternative<map<string, Var>::const_iterator>(it))
                return get<map<string, Var>::const_iterator>(it)->second;
            return *get<const Var*>(it);
        }

        iterator& operator++() {
            visit([](auto& i) { ++i; }, it);
            return *this;
        }

        bool operator==(const iterator& other) const {
            return it == other.it;
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }
    };

    iterator begin() const {
        if (is_list())   return get<vector<Var>>(data).cbegin();
        if (is_dict())   return get<map<string, Var>>(data).cbegin();
        return this;
    }

    iterator end() const {
        if (is_list())   return get<vector<Var>>(data).cend();
        if (is_dict())   return get<map<string, Var>>(data).cend();
        return this + 1;
    }

    friend ostream& operator<<(ostream& os, const Var& var);
};

ostream& operator<<(ostream& os, const Var& var) {
    visit([&](const auto& val) {
        using T = decay_t<decltype(val)>;
        if constexpr (is_same_v<T, int>) os << val;
        else if constexpr (is_same_v<T, double>)
            os << fixed << setprecision(2) << val;
        else if constexpr (is_same_v<T, complex<double>>)
            os << val.real() << "+" << val.imag() << "i";
        else if constexpr (is_same_v<T, vector<Var>>) {
            os << "[";
            for (size_t i = 0; i < val.size(); ++i) { if (i)os << ", "; os << val[i]; }
            os << "]";
        }
        else if constexpr (is_same_v<T, map<string, Var>>) {
            os << "{";
            bool first = true;
            for (auto& [k, v] : val) { if (!first)os << ", "; first = false; os << "\"" << k << "\": " << v; }
            os << "}";
        }
        else if constexpr (is_same_v<T, string>) os << "\"" << val << "\"";
    }, var.data);
    return os;
}

int main() {
    Var v = { 1, "hello", 2.0, complex<double>(1.0,2.0),Var{1, "hello", 2.0, complex<double>(1.0,2.0)} };
    cout << v << endl;
    cout << "-----------------------" << endl;
    cout << "列表 size = " << v.size() << endl;
    for (auto& d : v)
        cout << d << endl;

    Var num = 100;
    cout << "\nint size = " << num.size() << endl;
    for (auto& d : num)
        cout << d << endl;

    Var dict;
    dict.set("name", "Lele");
    dict.set("age", 10);
    cout << "\n字典 size = " << dict.size() << endl;
    cout << "字典内容: " << dict << endl;

    cout << "字典迭代：" << endl;
    for (auto& d : dict) {
        cout << d << endl;
    }

    cout << "\n=======================" << endl;
    cout << "类型检查演示：" << endl;
    cout << "num 类型: " << num.type() << endl;
    cout << "v 类型: " << v.type() << endl;
    cout << "dict 类型: " << dict.type() << endl;

    // ====================== 修复：全部 complex<double> ======================
    Var vvv = { "hello",1,1, {1,"hello", {"hello","gagaga"}},"hello", 2.0, complex<double>(1.0,2.0),Var{1, "hello", 2.0, complex<double>(1.0,2.0)}};
    cout << "vvv is " << vvv.type() << " = " << vvv << endl;

    vvv[0] = complex<double>(1, 2.0);
    cout << "vvv is " << vvv.type() << " = " << vvv << endl;

    vvv[0][0][0][0][0][0][0][0] = 1.0;
    cout << "vvv is " << vvv.type() << " = " << vvv << endl;

    vvv = "Ooo!";
    cout << "vvv is " << vvv.type() << " = " << vvv << endl;

    // ====================== 修复：这里原来是 vvv.type() 写错了！ ======================
    Var vvv1 = { 1,{1,{1,{"hello",{"gagaga",complex<double>(1,2),3.0,complex<double>(1,2)}}}}};
    cout << "vvv1 is " << vvv1.type() << " = " << vvv1 << endl;

    vvv1 = complex<double>(2, 3);
    cout << "vvv1 is " << vvv1.type() << " = " << vvv1 << endl;

    return 0;
}
