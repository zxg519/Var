# Var
C++万能数据结构！
data structure to store everything

## 参考代码 ##
```cpp
int main()
{
  Var null;
  Var a = {1,2,3};
  a+="hello,world";
  a[0] = 2026;
 
  Var b = {"gagaga",1,2,3,{1,2,3}};
  cout<<"b="<<b<<" with type:"<<b.type()<<endl;
  b = "hi";
  cout<<"b="<<b<<" with type:"<<b.type()<<endl;
  
  Var list1 = {1,2,3};
  Var list2={"a",'b',complex<double>(1,2)}
  Var list3 = list1 + list2;
  list3+=null;
  cout<<list3<<endl;
}
```

