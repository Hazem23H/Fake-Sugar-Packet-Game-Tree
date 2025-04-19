#pragma once
#include <vector>

template <typename T>
class Stack {
private:
    std::vector<T> elements;

public:
    // Push an element onto the stack
    void push(const T& element) {
        elements.push_back(element);
    }

    // Pop an element from the stack
    T pop() {
        if (isEmpty()) {
            throw std::out_of_range("Stack is empty");
        }
        T top = elements.back();
        elements.pop_back();
        return top;
    }

    // Peek at the top element without removing it
    T peek() const {
        if (isEmpty()) {
            throw std::out_of_range("Stack is empty");
        }
        return elements.back();
    }

    // Check if the stack is empty
    bool isEmpty() const {
        return elements.empty();
    }

    // Get the size of the stack
    size_t size() const {
        return elements.size();
    }

    // Clear all elements from the stack
    void clear() {
        elements.clear();
    }
};