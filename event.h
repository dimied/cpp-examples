#ifndef __EVENT__
#define __EVENT__

template < typename T >
class Event {
public:
     Event() {}
     Event(T t) : data(t) {}

    T getData() const {
        return data;
    }

    Event<T>& operator=(Event<T>& e) {
        this->data = e.data;
        return *this;
    }

    T operator()() const {
        return data+1;
    }

private:
    T data;
};

#endif