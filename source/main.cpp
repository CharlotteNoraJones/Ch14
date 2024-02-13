// Ch14 Ranges Notes

// 14.1 Introduction
// std lib offers args both constrained (with concepts) and unconstrained
// constrained versions are in <ranges>

// a range can be a...
// begin() and end() pair
// begin() and n, where n is number of elements
// {begin, pred}, where pred is a function where pred(p) begin true marks the end of the range

// range concept lets us say sort(v) instead of sort(v.begin(),v.end())

// example using ranges
#include <ranges>
using namespace std; // don't do this in real programs
using namespace ranges;
// example using ranges
template<forward_range R>
    requires sortable<iterator_t<R>>
void my_sort(R& r)
{
    return my_sort(r.begin(),end()); //1994 style sort
}

// using ranges has optimization advantages and helps to avoid silly errors from mismatched r.begin() and r.end()

// different kinds of iterators have different kinds of ranges

// 14.2 Views
// view: means of looking at a range

void user1(forward_range auto& r)
{
    filter_view v {r, [](int x) {return x%2;}}; // view (only) odd numbers from r

    cout << "odd numbers: "
    for (int x : v)
        cout << x << '\n';
}

// we read from the range of a filter view

// many ranges are infinte, but we might only want a few values
// there are thus views for just taking a few values from a range

void user2(forward_range auto& r)
{
    filter_view v{r, [](int x) {return x%2;}}; // view (only) odd numbers in r
    take_view tv {v, 100}; // view at most 100 element from v

    cout << "odd numbers: "
    for (int x : tv)
        cout << x << ' ';
}

// we can also use take_view directly

for (int x : take_view{v, 3})
    cout << x << ' ';

// we can use filter_view directly too
for (int x : take_view{ filter_view {r, [](int x) {return x % 2; }}, 3})
    cout << x << ' ';

// views are also known as range adaptors

// view interface is similar to range's interface. We can usually use a view whenever we can use a range

// a view doesn't own its elements, it's not responsible for deleting them
// view's shouldn't outlive their range

#include <vector>

auto bad()
{
    vector v {1, 2, 3, 4, 5};
    return filter_view{v,odd}; // v will be destroyed before the view
}

// views are cheap to copy, so pass them by value

// we can have views are user-defined types as well
struct Reading {
    int location {};
    int temperature {};
    int humidity {};
    int air_pressure {};
    // ..
}

int average_temp(vector<Reading> readings)
{
    if (readings.size()==0) throw No_readings{};
    double s {0};
    for (int x : views::elements<1>(readings)) // look at just the temperatures
        s += x;
    return s/readings.size();
}

// 14.3 Generators
// std provides a few factories for generating ranges on the flow

// using iota_view to generate a simple sequence
for(int x : iota_view(45,52)) // 42 43 44 45 46 47 48 49 50 51
    cout << x << ' ';

// istream_view gives a simple way to use istreams in range-for loops
for (auto x : istream_view<complex<double>(cin))
    cout << x << '\n';

// an istream_view can be composed with other views

auto cplx = istream_view<complex<double>>(cin);

for (auto x : transform_view(cplx, [](auto z){return z*z}))
    cout << x << '\n';

// an input of 1 2 3 produces 1 4 9

// 14.4 Pipelines
// std provides a function that makes a filter for each view
// an object can be used as an arg to the filter operator |
// example: filter() yields filter_view
// this lets us combine filters as a sequence

void user(forward_range auto& r)
{
    auto odd = [](int x) {return x % 2; };
    for (int x : r | view::filter(odd) | views::take(3))
        cout << x << ' ';
}

// An input range 2 4 6 8 20 produces 1 2 3.

// | works from left to right

// filter functions are in namespace ranges::views

void user(forward_range auto& r)
{
    for (int x : r | views::filter([](int x) {return x % 2}) | views::take(3))
        cout << x << ' ';
}

// we can also employ using namespace

void user(forward_range auto& r)
{
    using namespace views;

    auto odd = [](int x) {return x % 2;};

    for (int x : r | filter(odd) | take(3) )
        cout << x << ' ';
}

// implementation of views and pipelines involves a lot of very difficult template metaprogramming

// here is the conventional workaround to relying on views and pipes

void user(forward_range auto& r)
{
    int count = 0;
    for (int x : r)
        if (x % 2) {
            cout << x << ' ';
            if (++count == 3) return;
        }
    // the logic is a bit obscured here, however
}

// 14.5 Concepts Overview

// types on concepts
// concepts defining properties of types
// concepts defining iterators
// concepts defining ranges

// 14.5.1 Type Concepts
// common_with<X,Y> lets us make sure the mix of number types can be added, subtracted, etc without error
// if common_with<X,Y> is true, we can use common_type_t<X,Y> to compare a X with a Y by first converting both
// to common_type_t<X,Y>

common_type<string, const char*> s1 = some_fct()
common_type<string, const char*> s2 = some_other_fct();

if (s1<s2) {
    // ..
}

using common_type_t<Bigint,long> = Bigint; // for a suitable definition of Bigint

// there is no standard boolean concept, so here is a version of one
template<typename B>
concept Boolean =
    requires(B x, B y)
        {
            { x = true };
            { x = false };
            { x = (x == y) };
            { x = (x != y) };
            { x = !x };
            { x = (x = y) };
        };

// ideal for types is regular. 
// regular types works a lot like an int, has simple rules. 
// lacking a default == means most classes start out as semiregular

// when we pass an operation as a constrained template argument, we need to specify how it can be called, and sometimes assumptions about semantics
// a function is equality presevering if x==y imples f(x)==f(y)
// an equivalence relationship is reflexive, symmetric, and transitive
// relation and strict_weak_ordering only differ semantically
// std lib usually assume strict_weak_ordering for comparision

// 14.5.2 Iterator Concepts
// traditional standard algorithms access data through iterators, so there are concept to classify properties of iterator types

// basic idea of a sentinel is that we can iterate over a range starting at an iterator until the predicate becomes true for the element
// that way, an iterator p and a sentinel s define a range [p:s(*p)
// we could thus define a predicate for a sentinel for traversing a C-style string using a pointer as the iterator

// example
template<class Iter>
class Sentinel {
    public:
        Sentinel(int ee) : end(ee) { }
        Sentinel() : end(0) {} // concept sentinel_for requires a default constructor

        friend bool operator==(const Iter& p, Sentinel s) { return (*p == s.end); }
        friend bool operator!=(const Iter& p, Sentinel s) {return !(p == s);}
    private:
        iter_value_t<const char*> end; // the sentinel value
};

// friend declarator enables defining the == and != binary functions for comparing an iterator to a sentinel within the scope of the class

// checking that Sentinel meets the requirements of sentinel_for for a const char*:

static_assert(sentinel_for<Sentinel<const char*>,const char*>); // check the Sentinel for C-style strings

// this code uses the code above to write "Hello World"

const char aa[] = "Hello, World!\nBye for now\n";

ranges::for_each(aa, Sentinel<const char*>('\n'),[](const char x) {cout << x;});

// 14.5.3 Range Concepts
// range concepts define the properties of ranges
// range concepts enable overloading of implementations based on type properties of their inputs