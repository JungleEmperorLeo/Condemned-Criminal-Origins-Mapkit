#pragma once
// Limited containers for legacy pointer/scalar/POD arrays. Stripifier uses no
// erase, ordered-set traversal, nontrivial vector values, or list-node ops.
#ifndef NULL
#define NULL 0
#endif
void* operator new(unsigned int);
void* operator new[](unsigned int);
void operator delete(void*);
void operator delete[](void*);
extern "C" void* __cdecl memset(void*,int,unsigned int);
extern "C" void* __cdecl memcpy(void*,const void*,unsigned int);
namespace NvCompat {
template<class T> inline T Max(T a,T b) {return a>b?a:b;}
template<class T> class Vector {
    T* values;unsigned int length,capacity;
public:
    typedef T* iterator;
    Vector():values(0),length(0),capacity(0) {}
    explicit Vector(unsigned int n):values(0),length(0),capacity(0) {resize(n);}
    Vector(const Vector& rhs):values(0),length(0),capacity(0) {*this=rhs;}
    ~Vector() {delete[] values;}
    Vector& operator=(const Vector& rhs) {if(this!=&rhs){resize(rhs.length);for(unsigned int i=0;i<length;++i) values[i]=rhs.values[i];}return *this;}
    void reserve(unsigned int n) {if(n<=capacity)return;T* replacement=new T[n];for(unsigned int i=0;i<length;++i) replacement[i]=values[i];delete[] values;values=replacement;capacity=n;}
    void resize(unsigned int n) {reserve(n);for(unsigned int i=length;i<n;++i) values[i]=T();length=n;}
    void push_back(const T& value) {if(length==capacity)reserve(capacity?capacity*2:4);values[length++]=value;}
    void clear() {length=0;}
    unsigned int size() const {return length;}
    T* data() {return values;}
    const T* data() const {return values;}
    iterator begin() {return values;}
    iterator end() {return values+length;}
    const T* begin() const {return values;}
    const T* end() const {return values+length;}
    T& operator[](unsigned int i) {return values[i];}
    const T& operator[](unsigned int i) const {return values[i];}
    bool empty() const {return !length;}
    template<class Iter> void assign(Iter first,Iter last) {resize((unsigned int)(last-first));for(unsigned int i=0;i<length;++i)values[i]=first[i];}
};
template<class T> class Set {
    Vector<T> items;
public:
    typedef T* iterator;
    iterator end() {return items.end();}
    iterator find(const T& value) {for(iterator p=items.begin();p!=items.end();++p)if(*p==value)return p;return end();}
    void insert(const T& value) {if(find(value)==end())items.push_back(value);}
};
}
