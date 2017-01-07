// Submitter: justinic(Choi, Justin)
// Partner  : brgallar(Gallardo, Brayan)
// We certify that we worked cooperatively on this programming
//   assignment, according to the rules for pair programming
#ifndef HEAP_PRIORITY_QUEUE_HPP_
#define HEAP_PRIORITY_QUEUE_HPP_

#include <string>
#include <iostream>
#include <sstream>
#include <initializer_list>
#include "ics_exceptions.hpp"
#include <utility>              //For std::swap function
#include "array_stack.hpp"      //See operator <<


namespace ics {


#ifndef undefinedgtdefined
#define undefinedgtdefined
template<class T>
bool undefinedgt (const T& a, const T& b) {return false;}
#endif /* undefinedgtdefined */

//Instantiate the templated class supplying tgt(a,b): true, iff a has higher priority than b.
//If tgt is defaulted to undefinedgt in the template, then a constructor must supply cgt.
//If both tgt and cgt are supplied, then they must be the same (by ==) function.
//If neither is supplied, or both are supplied but different, TemplateFunctionError is raised.
//The (unique) non-undefinedgt value supplied by tgt/cgt is stored in the instance variable gt.
template<class T, bool (*tgt)(const T& a, const T& b) = nullptr> class HeapPriorityQueue {
  public:
    typedef bool (*gtfunc) (const T& a, const T& b);

    //Destructor/Constructors
    ~HeapPriorityQueue();

    HeapPriorityQueue(bool (*cgt)(const T& a, const T& b) = nullptr);
    explicit HeapPriorityQueue(int initial_length, bool (*cgt)(const T& a, const T& b) = nullptr);
    HeapPriorityQueue(const HeapPriorityQueue<T,tgt>& to_copy, bool (*cgt)(const T& a, const T& b) = nullptr);
    explicit HeapPriorityQueue(const std::initializer_list<T>& il, bool (*cgt)(const T& a, const T& b) = nullptr);

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    explicit HeapPriorityQueue (const Iterable& i, bool (*cgt)(const T& a, const T& b) = nullptr);


    //Queries
    bool empty      () const;
    int  size       () const;
    T&   peek       () const;
    std::string str () const; //supplies useful debugging information; contrast to operator <<


    //Commands
    int  enqueue (const T& element);
    T    dequeue ();
    void clear   ();

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    int enqueue_all (const Iterable& i);


    //Operators
    HeapPriorityQueue<T,tgt>& operator = (const HeapPriorityQueue<T,tgt>& rhs);
    bool operator == (const HeapPriorityQueue<T,tgt>& rhs) const;
    bool operator != (const HeapPriorityQueue<T,tgt>& rhs) const;

    template<class T2, bool (*gt2)(const T2& a, const T2& b)>
    friend std::ostream& operator << (std::ostream& outs, const HeapPriorityQueue<T2,gt2>& pq);



    class Iterator {
      public:
        //Private constructor called in begin/end, which are friends of HeapPriorityQueue<T,tgt>
        ~Iterator();
        T           erase();
        std::string str  () const;
        HeapPriorityQueue<T,tgt>::Iterator& operator ++ ();
        HeapPriorityQueue<T,tgt>::Iterator  operator ++ (int);
        bool operator == (const HeapPriorityQueue<T,tgt>::Iterator& rhs) const;
        bool operator != (const HeapPriorityQueue<T,tgt>::Iterator& rhs) const;
        T& operator *  () const;
        T* operator -> () const;
        friend std::ostream& operator << (std::ostream& outs, const HeapPriorityQueue<T,tgt>::Iterator& i) {
          outs << i.str(); //Use the same meaning as the debugging .str() method
          return outs;
        }

        friend Iterator HeapPriorityQueue<T,tgt>::begin () const;
        friend Iterator HeapPriorityQueue<T,tgt>::end   () const;

      private:
        //If can_erase is false, the value has been removed from "it" (++ does nothing)
        HeapPriorityQueue<T,tgt>  it;                 //copy of HPQ (from begin), to use as iterator via dequeue
        HeapPriorityQueue<T,tgt>* ref_pq;
        int                       expected_mod_count;
        bool                      can_erase = true;

        //Called in friends begin/end
        //These constructors have different initializers (see it(...) in first one)
        Iterator(HeapPriorityQueue<T,tgt>* iterate_over, bool from_begin);    // Called by begin
        Iterator(HeapPriorityQueue<T,tgt>* iterate_over);                     // Called by end
    };


    Iterator begin () const;
    Iterator end   () const;


  private:
    bool (*gt) (const T& a, const T& b); //The gt used by enqueue (from template or constructor)
    T*  pq;                              //Array represents a heap, so it uses the heap ordering property
    int length    = 0;                   //Physical length of array: must be >= .size()
    int used      = 0;                   //Amount of array used:  invariant: 0 <= used <= length
    int mod_count = 0;                   //For sensing concurrent modification


    //Helper methods
    void ensure_length  (int new_length);
    int  left_child     (int i) const;         //Useful abstractions for heaps as arrays
    int  right_child    (int i) const;
    int  parent         (int i) const;
    bool is_root        (int i) const;
    bool in_heap        (int i) const;
    void percolate_up   (int i);
    void percolate_down (int i);
    void heapify        ();                   // Percolate down all value is array (from indexes used-1 to 0): O(N)
  };





////////////////////////////////////////////////////////////////////////////////
//
//HeapPriorityQueue class and related definitions

//Destructor/Constructors

template<class T, bool (*tgt)(const T& a, const T& b)>
HeapPriorityQueue<T,tgt>::~HeapPriorityQueue() {
    delete[] pq;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
HeapPriorityQueue<T,tgt>::HeapPriorityQueue(bool (*cgt)(const T& a, const T& b))
:   gt(tgt != nullptr ? tgt : cgt)
{
    if (gt == nullptr){
        throw TemplateFunctionError("HeapPriorityQueue::default constructor: neither specified");
    }
    if (tgt != nullptr and cgt != nullptr and tgt != cgt){
        throw TemplateFunctionError("HeapPriorityQueue::default constructor: both specified and different");
    }
    pq = new T[length];
}


template<class T, bool (*tgt)(const T& a, const T& b)>
HeapPriorityQueue<T,tgt>::HeapPriorityQueue(int initial_length, bool (*cgt)(const T& a, const T& b))
:   gt(tgt != nullptr ? tgt : cgt), length(initial_length)
{
    if (gt == nullptr){
        throw TemplateFunctionError("HeapPriorityQueue::length constructor: neither specified");
    }
    if (tgt != nullptr and cgt != nullptr and tgt != cgt){
        throw TemplateFunctionError("HeapPriorityQueue::length constructor: both specified and different");
    }
    if (length < 0){
        length = 0;
    }
    pq = new T[length];
}


template<class T, bool (*tgt)(const T& a, const T& b)>
HeapPriorityQueue<T,tgt>::HeapPriorityQueue(const HeapPriorityQueue<T,tgt>& to_copy, bool (*cgt)(const T& a, const T& b))
: gt (tgt != nullptr ? tgt : cgt), length(to_copy.length)
{
    if (gt == nullptr){
        gt = to_copy.gt;
    }
    if (tgt != nullptr and cgt != nullptr and tgt != cgt){
        throw TemplateFunctionError("HeapPriorityQueue::copy constructor: both specified and different");
    }
    pq = new T[length];
    used = to_copy.used;
    for (int i = 0; i < used; i++){
        pq[i] = to_copy.pq[i];
    }
    if (gt != to_copy.gt){
        for (int i = 0; i < used; i++) {
            enqueue(to_copy.pq[i]);
        }
    }
}

// FIX IF NOT WORKING
template<class T, bool (*tgt)(const T& a, const T& b)>
HeapPriorityQueue<T,tgt>::HeapPriorityQueue(const std::initializer_list<T>& il, bool (*cgt)(const T& a, const T& b))
:   gt(tgt != nullptr ? tgt : cgt), length(il.size())
{
    if (gt == nullptr){
        throw TemplateFunctionError("HeapPriorityQueue::initializer_list constructor: neither specified");
    }
    if (tgt != nullptr and cgt != nullptr and tgt != cgt){
        throw TemplateFunctionError("HeapPriorityQueue::initializer_list constructor: both specified and different");
    }
    pq = new T[length];
    int i = 0;

    for (const T& pq_elem : il){
        enqueue(pq_elem);
    }
}


template<class T, bool (*tgt)(const T& a, const T& b)>
template<class Iterable>
HeapPriorityQueue<T,tgt>::HeapPriorityQueue(const Iterable& i, bool (*cgt)(const T& a, const T& b))
:   gt(tgt != nullptr ? tgt : cgt), length(i.size())
{
    if (gt == nullptr){
        throw TemplateFunctionError("HeapPriorityQueue::Iterable constructor: neither specified");
    }
    if (tgt != nullptr and cgt != nullptr and tgt != cgt){
        throw TemplateFunctionError("HeapPriorityQueue::Iterable constructor: both specified and different");
    }
    pq = new T[length];
    int j = 0;
    for (const T& v : i){
        enqueue(v);
    }
}


////////////////////////////////////////////////////////////////////////////////
//
//Queries

template<class T, bool (*tgt)(const T& a, const T& b)>
bool HeapPriorityQueue<T,tgt>::empty() const {
    return used == 0;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
int HeapPriorityQueue<T,tgt>::size() const {
    return used;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
T& HeapPriorityQueue<T,tgt>::peek () const {
    if (empty()){
        throw EmptyError("HeapPriorityQueue::peek");
    }
    return pq[0];
}


template<class T, bool (*tgt)(const T& a, const T& b)>
std::string HeapPriorityQueue<T,tgt>::str() const {
    std::ostringstream answer;
    answer << "HeapPriorityQueue[";
    if (length != 0){
        answer << "0:" << pq[0];
        for (int i = 1; i < length; i++){
            answer << "," << i << ":" << pq[i];
        }
    }
    answer << "](length=" << length << ",used=" << used << ",mod_count=" << mod_count << ")";
    return answer.str();
}


////////////////////////////////////////////////////////////////////////////////
//
//Commands
//fix
template<class T, bool (*tgt)(const T& a, const T& b)>
int HeapPriorityQueue<T,tgt>::enqueue(const T& element) {
    this -> ensure_length(used + 1);
    pq[used++] = element;
    percolate_up(used - 1);
    mod_count++;
    return 1;
}

template<class T, bool (*tgt)(const T& a, const T& b)>
T HeapPriorityQueue<T,tgt>::dequeue() {
    if (this -> empty()){
        throw EmptyError("HeapPriorityQueue::dequeue");
    }
    T to_return = pq[0];
    pq[0] = pq[--used];
    mod_count++;
    percolate_down(0);
    this->ensure_length(used);
    return to_return;

}


template<class T, bool (*tgt)(const T& a, const T& b)>
void HeapPriorityQueue<T,tgt>::clear() {
    used = 0;
    mod_count++;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
template <class Iterable>
int HeapPriorityQueue<T,tgt>::enqueue_all (const Iterable& i) {
    int count = 0;
    for (const T& v : i){
        count += enqueue(v);
    }
    return count;
}


////////////////////////////////////////////////////////////////////////////////
//
//Operators

template<class T, bool (*tgt)(const T& a, const T& b)>
HeapPriorityQueue<T,tgt>& HeapPriorityQueue<T,tgt>::operator = (const HeapPriorityQueue<T,tgt>& rhs) {
    if (this == &rhs){
        return *this;
    }
    gt = rhs.gt;
    this -> ensure_length(rhs.used);
    used = rhs.used;
    for (int i = 0; i < used; i++){
        pq[i] = rhs.pq[i];
    }
    mod_count++;
    return *this;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
bool HeapPriorityQueue<T,tgt>::operator == (const HeapPriorityQueue<T,tgt>& rhs) const {
    if (this == &rhs){
        return true;
    }
    if (gt != rhs.gt){
        return false;
    }
    if (used != rhs.size()){
        return false;
    }
    HeapPriorityQueue<T, tgt>::Iterator left = this -> begin(), right = rhs.begin();
    int i = 0;
    while (i < used){
        if (*left == *right){
            return true;
        }
        else if(*left != *right){
            return *left != *right;
        }
        i++;
        left++;
        right++;
    }
    return true;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
bool HeapPriorityQueue<T,tgt>::operator != (const HeapPriorityQueue<T,tgt>& rhs) const {
    return !(*this == rhs);
}


template<class T, bool (*tgt)(const T& a, const T& b)>
std::ostream& operator << (std::ostream& outs, const HeapPriorityQueue<T,tgt>& p) {
    outs << "priority_queue[";

    if (!p.empty()) {
        outs << p.pq[0];
        for (int i=1; i<p.used; ++i)
            outs << ","<< p.pq[i];
    }

    outs << "]:highest";
    return outs;
}


////////////////////////////////////////////////////////////////////////////////
//
//Iterator constructors


    //change used - 1 to true if error
template<class T, bool (*tgt)(const T& a, const T& b)>
auto HeapPriorityQueue<T,tgt>::begin () const -> HeapPriorityQueue<T,tgt>::Iterator {
    return Iterator(const_cast<HeapPriorityQueue<T, tgt>*>(this), used - 1);
}

// remove -1 if error
template<class T, bool (*tgt)(const T& a, const T& b)>
auto HeapPriorityQueue<T,tgt>::end () const -> HeapPriorityQueue<T,tgt>::Iterator {
    return Iterator(const_cast<HeapPriorityQueue<T, tgt>*>(this));
}


////////////////////////////////////////////////////////////////////////////////
//
//Private helper methods

template<class T, bool (*tgt)(const T& a, const T& b)>
void HeapPriorityQueue<T,tgt>::ensure_length(int new_length) {
        if (length < new_length) {
            T* temp = pq;
            length = std::max(new_length, int(length + 1));
            pq = new T[length];
            for (int i = 0; i < this -> size(); i++) {
                pq[i] = temp[i];
            }
        }
        else{
            return;
        }
    return;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
int HeapPriorityQueue<T,tgt>::left_child(int i) const {
    return int(2 * i + 1);
}

template<class T, bool (*tgt)(const T& a, const T& b)>
int HeapPriorityQueue<T,tgt>::right_child(int i) const {
    return int(2 * i + 2);
}

template<class T, bool (*tgt)(const T& a, const T& b)>
int HeapPriorityQueue<T,tgt>::parent(int i) const {
    return int((i-1)/2);
}

template<class T, bool (*tgt)(const T& a, const T& b)>
bool HeapPriorityQueue<T,tgt>::is_root(int i) const {
    return i == 0;
}

template<class T, bool (*tgt)(const T& a, const T& b)>
bool HeapPriorityQueue<T,tgt>::in_heap(int i) const {
    return i < used;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
void HeapPriorityQueue<T,tgt>::percolate_up(int i) {
    if (is_root(i)){
        return;
    }
    if (!is_root(i)) {
        int temp = parent(i);
        if (gt(pq[i], pq[temp])) {
            std::swap(pq[i], pq[temp]);
            percolate_up(temp);
        }
    }
    return;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
void HeapPriorityQueue<T,tgt>::percolate_down(int i) {
    while (in_heap(left_child(i))){
        int max_child;
        if (gt(pq[left_child(i)], pq[right_child(i)])){
            max_child = left_child(i);
        }
        else{
            max_child = right_child(i);
        }
        if (gt(pq[i], pq[max_child])){
            break;
        }
        std::swap(pq[i], pq[max_child]);
        i = max_child;
    }
    return;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
void HeapPriorityQueue<T,tgt>::heapify() {
    for (int i = used; i > 0; i--){
        int parent = i;
        percolate_down(parent);
    }
    return;
}


////////////////////////////////////////////////////////////////////////////////
//
//Iterator class definitions

template<class T, bool (*tgt)(const T& a, const T& b)>
HeapPriorityQueue<T,tgt>::Iterator::Iterator(HeapPriorityQueue<T,tgt>* iterate_over, bool tgt_nullptr)
:   it(*iterate_over, iterate_over -> gt), ref_pq(iterate_over), expected_mod_count(iterate_over -> mod_count)
// HeapPriorityQueue<T, tgt> it;    ref_pq = <T,tgt>*,
{
}


template<class T, bool (*tgt)(const T& a, const T& b)>
HeapPriorityQueue<T,tgt>::Iterator::Iterator(HeapPriorityQueue<T,tgt>* iterate_over)
:   it (iterate_over -> gt), ref_pq(iterate_over), expected_mod_count(iterate_over -> mod_count)
{
}


template<class T, bool (*tgt)(const T& a, const T& b)>
HeapPriorityQueue<T,tgt>::Iterator::~Iterator() {

}


template<class T, bool (*tgt)(const T& a, const T& b)>
T HeapPriorityQueue<T,tgt>::Iterator::erase() {
    if (expected_mod_count != ref_pq -> mod_count){
        throw ConcurrentModificationError("HeapPriorityQueue::Iterator::erase");
    }
    if (!can_erase){
        throw CannotEraseError("HeapPriorityQueue::Iterator::erase Iterator cursor already erased");
    }
    if (it.empty()){
        throw CannotEraseError("HeapPriorityQueue::Iterator::erase Iterator cursor beyond data structure");
    }

    can_erase = false;
    T to_return = it.dequeue();

    return to_return;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
std::string HeapPriorityQueue<T,tgt>::Iterator::str() const {
    std::ostringstream answer;
    answer << ref_pq->str()  <<  "/expected_mod_count=" << expected_mod_count << "/can_erase=" << can_erase;
    return answer.str();
}


template<class T, bool (*tgt)(const T& a, const T& b)>
auto HeapPriorityQueue<T,tgt>::Iterator::operator ++ () -> HeapPriorityQueue<T,tgt>::Iterator& {
    if (expected_mod_count != ref_pq -> mod_count){
        throw ConcurrentModificationError("HeapPriorityQueue::Iterator::operator ++");
    }
    if (it.size() == 0){
        return *this;
    }
    if (can_erase == false){
        can_erase = true;
    }
    else{
        it.dequeue();
    }
    return *this;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
auto HeapPriorityQueue<T,tgt>::Iterator::operator ++ (int) -> HeapPriorityQueue<T,tgt>::Iterator {
    if (expected_mod_count != ref_pq -> mod_count){
        throw ConcurrentModificationError("HeapPriorityQueue::Iterator::operator ++ (int)");
    }
    if (it.size() == 0){
        return *this;
    }

    Iterator to_return(*this);
    if (can_erase == false){
        can_erase = true;
    }
    else{
        it.dequeue();
    }
    return to_return;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
bool HeapPriorityQueue<T,tgt>::Iterator::operator == (const HeapPriorityQueue<T,tgt>::Iterator& rhs) const {
    const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
    if (rhsASI == 0){
        throw IteratorTypeError("HeapPriorityQueue::Iterator::operator ==");
    }
    if (expected_mod_count != ref_pq -> mod_count){
        throw ConcurrentModificationError("HeapPriorityQueue::Iterator::operator ==");
    }
    if (ref_pq != rhsASI -> ref_pq){
        throw ComparingDifferentIteratorsError("HeapPriorityQueue::Iterator::operator ==");
    }
    return this -> it.used == rhsASI -> it.used;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
bool HeapPriorityQueue<T,tgt>::Iterator::operator != (const HeapPriorityQueue<T,tgt>::Iterator& rhs) const {
    const Iterator* rhsASI = dynamic_cast<const Iterator*> (&rhs);
    if (rhsASI == 0){
        throw IteratorTypeError("HeapPriorityQueue::Iterator::operator !=");
    }
    if (expected_mod_count != ref_pq -> mod_count){
        throw ConcurrentModificationError("HeapPriorityQueue::Iterator::operator !=");
    }
    if (ref_pq != rhsASI -> ref_pq){
        throw ComparingDifferentIteratorsError("HeapPriorityQueue::Iterator::operator !=");
    }
    return this -> it.used != rhsASI -> it.used;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
T& HeapPriorityQueue<T,tgt>::Iterator::operator *() const {
    if (expected_mod_count != ref_pq -> mod_count){
        throw ConcurrentModificationError("HeapPriorityQueue::Iterator::operator *");
    }
    if (!can_erase || it.size() == 0){
        std::ostringstream where;
        where << " when size " << ref_pq -> size();
        throw IteratorPositionIllegal("HeapPriorityQueue::Iterator::operator * Iterator Illegal: " + where.str());
    }
    return it.peek();

}


template<class T, bool (*tgt)(const T& a, const T& b)>
T* HeapPriorityQueue<T,tgt>::Iterator::operator ->() const {
    if (expected_mod_count != ref_pq -> mod_count){
        throw ConcurrentModificationError("HeapPriorityQueue::Iterator::operator ->");
    }
    if (!can_erase || it.size() == 0){
        std::ostringstream where;
        where <<  " when size " << ref_pq -> size();
        throw IteratorPositionIllegal("HeapPriorityQueue::Iterator::operator -> Iterator illegal: " + where.str());
    }
    return it.peek();
}

}

#endif /* HEAP_PRIORITY_QUEUE_HPP_ */
