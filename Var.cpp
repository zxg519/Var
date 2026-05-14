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
#include <sstream>

using namespace std;

struct Var;

using VarVariant = variant<
    nullptr_t,
    int,
    double,
    char,
    complex<double>,
    string,
    vector<Var>,
    map<string, Var>
>;

struct Var {
private:
    VarVariant data;

public:
    Var() : data(nullptr) {}
    Var(int v) : data(v) {}
    Var(double v) : data(v) {}
    Var(char v) : data(v) {}
    Var(complex<double> v) : data(v) {}
    Var(const string& v) : data(v) {}
    // 修复1：const char*构造函数增加空指针校验
    Var(const char* v) {
        if (v == nullptr) {
            data = nullptr;
        } else {
            data = string(v);
        }
    }
    Var(const vector<Var>& v) : data(v) {}
    Var(const map<string, Var>& v) : data(v) {}
    Var(initializer_list<Var> v) : data(vector<Var>(v)) {}

    Var& operator=(int v) { data = v; return *this; }
    Var& operator=(double v) { data = v; return *this; }
    Var& operator=(char v) { data = v; return *this; }
    Var& operator=(complex<double> v) { data = v; return *this; }
    Var& operator=(const string& v) { data = v; return *this; }
    // 修复2：const char*赋值运算符增加空指针校验
    Var& operator=(const char* v) {
        if (v == nullptr) {
            data = nullptr;
        } else {
            data = string(v);
        }
        return *this;
    }
    Var& operator=(const vector<Var>& v) { data = v; return *this; }
    Var& operator=(const map<string, Var>& v) { data = v; return *this; }
    Var& operator=(initializer_list<Var> v) { data = vector<Var>(v); return *this; }

    string type() const {
        if (is_null())    return "null";
        if (is_char())    return "char";
        if (is_int())     return "int";
        if (is_double())  return "double";
        if (is_complex()) return "complex";
        if (is_string())  return "string";
        if (is_list())    return "list";
        if (is_dict())    return "dict";
        return "unknown";
    }

    bool is_null() const     { return holds_alternative<nullptr_t>(data); }
    bool is_char() const     { return holds_alternative<char>(data); }
    bool is_int() const      { return holds_alternative<int>(data); }
    bool is_double() const   { return holds_alternative<double>(data); }
    bool is_complex() const  { return holds_alternative<complex<double>>(data); }
    bool is_string() const   { return holds_alternative<string>(data); }
    bool is_list() const     { return holds_alternative<vector<Var>>(data); }
    bool is_dict() const     { return holds_alternative<map<string, Var>>(data); }

    bool is_numeric() const {
        return is_int() || is_double() || is_char();
    }

    Var operator+(const Var& other) const {
        if (is_null() && !other.is_null())
            return other;
        if (!is_null() && other.is_null())
            return *this;
        if (is_null() && other.is_null())
            return Var();

        if (is_list() && other.is_list()) {
            auto res = get<vector<Var>>(data);
            for (const auto& item : get<vector<Var>>(other.data))
                res.push_back(item);
            return res;
        }

        if (is_list() && !other.is_list() && !other.is_dict()) {
            auto res = get<vector<Var>>(data);
            res.push_back(other);
            return res;
        }
        if (other.is_list() && !is_list() && !other.is_dict()) {
            vector<Var> res;
            res.push_back(*this);
            for (const auto& item : get<vector<Var>>(other.data))
                res.push_back(item);
            return res;
        }

        if (is_dict() && other.is_dict()) {
            auto res = get<map<string, Var>>(data);
            for (const auto& [k, v] : get<map<string, Var>>(other.data))
                res[k] = v;
            return res;
        }

        if (is_string() || other.is_string()) {
            stringstream ss;
            visit([&](const auto& x) { ss << x; }, data);
            visit([&](const auto& x) { ss << x; }, other.data);
            return Var(ss.str());
        }

        if (is_numeric() && other.is_numeric()) {
            if (is_double() || other.is_double()) {
                auto get_val = [](const Var& x) {
                    if (x.is_double()) return get<double>(x.data);
                    if (x.is_int()) return (double)get<int>(x.data);
                    return (double)get<char>(x.data);
                };
                return Var(get_val(*this) + get_val(other));
            }

            if (is_int() && other.is_int())
                return Var(get<int>(data) + get<int>(other.data));

            if (is_char() && other.is_char())
                return Var((char)(get<char>(data) + get<char>(other.data)));
        }

        if (is_complex() || other.is_complex()) {
            auto to_c = [](const Var& x) {
                if (x.is_complex()) return get<complex<double>>(x.data);
                if (x.is_double()) return complex<double>(get<double>(x.data));
                if (x.is_int()) return complex<double>(get<int>(x.data));
                return complex<double>(get<char>(x.data));
            };
            return Var(to_c(*this) + to_c(other));
        }

        return Var();
    }

    Var& operator+=(const Var& other) {
        *this = *this + other;
        return *this;
    }

public:
    size_t size() const {
        if (is_list()) return get<vector<Var>>(data).size();
        if (is_dict()) return get<map<string, Var>>(data).size();
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

        variant<vector<Var>::const_iterator, map<string, Var>::const_iterator, const Var*> it;

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

        bool operator==(const iterator& other) const { return it == other.it; }
        bool operator!=(const iterator& other) const { return !(*this == other); }
    };

    iterator begin() const {
        if (is_list()) return get<vector<Var>>(data).cbegin();
        if (is_dict()) return get<map<string, Var>>(data).cbegin();
        return this;
    }

    iterator end() const {
        if (is_list()) return get<vector<Var>>(data).cend();
        if (is_dict()) return get<map<string, Var>>(data).cend();
        return this + 1;
    }

    friend ostream& operator<<(ostream& os, const Var& var);
};

ostream& operator<<(ostream& os, const Var& var) {
    visit([&](const auto& val) {
        using T = decay_t<decltype(val)>;
        if constexpr (is_same_v<T, nullptr_t>) os << "null";
        else if constexpr (is_same_v<T, char>) os << "'" << val << "'";
        else if constexpr (is_same_v<T, int>) os << val;
        else if constexpr (is_same_v<T, double>) os << fixed << setprecision(2) << val;
        else if constexpr (is_same_v<T, complex<double>>) os << val.real() << "+" << val.imag() << "i";
        else if constexpr (is_same_v<T, vector<Var>>) {
            os << "[";
            for (size_t i = 0; i < val.size(); ++i) {
                if (i) os << ", ";
                os << val[i];
            }
            os << "]";
        }
        else if constexpr (is_same_v<T, map<string, Var>>) {
            os << "{";
            bool first = true;
            for (auto& [k, v] : val) {
                if (!first) os << ", ";
                first = false;
                os << "\"" << k << "\": " << v;
            }
            os << "}";
        }
        else if constexpr (is_same_v<T, string>) os << "\"" << val << "\"";
    }, var.data);
    return os;
}

int main() {
    // ===================== 原有代码完全不动 =====================
    Var v = { 1, "hello", 2.0, complex<double>(1.0,2.0),Var{1, "hello", 2.0, complex<double>(1.0,2.0)} };
    cout << v << endl;
    cout << "-----------------------" << endl;
    cout << "列表 size = " << v.size() << endl;
    for (auto& d : v)
        cout << d << " (类型:" << d.type() << ")" << endl;

    Var num = 100;
    cout << "\nint size = " << num.size() << endl;
    for (auto& d : num) cout << d << endl;

    Var dict;
    dict.set("name", "Lele");
    dict.set("age", 10);
    cout << "\n字典 size = " << dict.size() << endl;
    cout << "字典内容: " << dict << endl;

    cout << "字典迭代：" << endl;
    for (auto& d : dict) cout << d << endl;

    cout << "\n=======================" << endl;
    cout << "类型检查演示：" << endl;
    cout << "num  类型: " << num.type() << endl;
    cout << "v    类型: " << v.type() << endl;
    cout << "dict 类型: " << dict.type() << endl;

    Var c1 = 'A';
    Var c2 = 'z';
    Var c3 = '5';
    cout << "\n=======================" << endl;
    cout << "c1 = " << c1 << " 类型: " << c1.type() << endl;
    cout << "c2 = " << c2 << " 类型: " << c2.type() << endl;
    cout << "c3 = " << c3 << " 类型: " << c3.type() << endl;

    Var mixed_list = { 100, 3.14, 'X', "Test", complex<double>(3,4), '!', 999, Var{'a','b','c'} };
    cout << "\n混合列表: " << mixed_list << endl;

    Var vvv = { "hello", 'A', 1, 1, {1,'Z',{"hello",'g'}},"hello", 2.0, complex<double>(1.0,2.0) };
    cout << "\nvvv = " << vvv << endl;

    cout << "\n=============================================" << endl;
    cout << "              null + 任何 测试               " << endl;
    cout << "=============================================" << endl;
    Var n;
    Var a = 100;
    Var b = 2.5;
    Var s = "test";
    Var lst = {1,2,3};
    cout << "null + 100 = " << (n + a) << "  类型:" << (n + a).type() << endl;
    cout << "100 + null = " << (a + n) << "  类型:" << (a + n).type() << endl;
    cout << "null + 2.5 = " << (n + b) << "  类型:" << (n + b).type() << endl;
    cout << "null + \"test\" = " << (n + s) << "  类型:" << (n + s).type() << endl;
    cout << "null + [1,2,3] = " << (n + lst) << "  类型:" << (n + lst).type() << endl;
    cout << "null + null = " << (n + n) << "  类型:" << (n + n).type() << endl;

    cout << "\n=============================================" << endl;
    cout << "         double 加法不会变 complex           " << endl;
    cout << "=============================================" << endl;
    Var d1 = 1.5, d2 = 2.5;
    auto res = d1 + d2;
    cout << d1 << " + " << d2 << " = " << res << "  类型:" << res.type() << endl;

    // ===================== 【你要的：追加 list 相关加法测试】 =====================
    cout << "\n=============================================" << endl;
    cout << "         list + double / double + list        " << endl;
    cout << "=============================================" << endl;
    Var list1 = { 10, 20, 30 };
    Var num_d = 1.5;

    cout << "list1 = " << list1 << endl;
    cout << "num_d = " << num_d << endl;
    cout << "list1 + num_d = " << (list1 + num_d) << "  类型:" << (list1 + num_d).type() << endl;
    cout << "num_d + list1 = " << (num_d + list1) << "  类型:" << (num_d + list1).type() << endl;

    cout << "\n=============================================" << endl;
    cout << "                 list + list                 " << endl;
    cout << "=============================================" << endl;
    Var list2 = { 'A', "test", 5.5 };
    cout << "list1 = " << list1 << endl;
    cout << "list2 = " << list2 << endl;
    cout << "list1 + list2 = " << (list1 + list2) << "  类型:" << (list1 + list2).type() << endl;

    cout << "\n=============================================" << endl;
    cout << "                list += 测试                 " << endl;
    cout << "=============================================" << endl;
    Var list_test = { 1,2,3 };
    cout << "origin: " << list_test << endl;

    list_test += 10.55;
    cout << "list_test += 10.55 → " << list_test << endl;

    list_test += Var{ 999, "abc" };
    cout << "list_test += [999, \"abc\"] → " << list_test << endl;

    list_test += nullptr;
    cout << "list_test += null → " << list_test << endl;

    return 0;
}
