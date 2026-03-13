/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<
    class Key,
    class T,
    class Compare = std::less<Key>
> class map {
public:
    /**
     * the internal type of data.
     * it should have a default constructor, a copy constructor.
     * You can use sjtu::map as value_type by typedef.
     */
    typedef pair<const Key, T> value_type;

private:
    enum Color { RED, BLACK };

    struct Node {
        value_type* data;
        Node* left;
        Node* right;
        Node* parent;
        Color color;

        Node(const value_type& val, Node* p = nullptr)
            : data(new value_type(val)), left(nullptr), right(nullptr), parent(p), color(RED) {}

        ~Node() {
            delete data;
        }
    };

    Node* root;
    size_t node_count;
    Compare comp;

    // Helper function to copy a tree
    Node* copy_tree(Node* other_node, Node* parent) {
        if (other_node == nullptr) {
            return nullptr;
        }
        Node* new_node = new Node(*other_node->data, parent);
        new_node->color = other_node->color;
        new_node->left = copy_tree(other_node->left, new_node);
        new_node->right = copy_tree(other_node->right, new_node);
        return new_node;
    }

    // Helper function to delete a tree
    void delete_tree(Node* node) {
        if (node == nullptr) return;
        delete_tree(node->left);
        delete_tree(node->right);
        delete node;
    }

    // Find minimum node in subtree
    Node* minimum(Node* node) const {
        if (node == nullptr) return nullptr;
        while (node->left != nullptr) {
            node = node->left;
        }
        return node;
    }

    // Find maximum node in subtree
    Node* maximum(Node* node) const {
        if (node == nullptr) return nullptr;
        while (node->right != nullptr) {
            node = node->right;
        }
        return node;
    }

    // Find successor of a node
    Node* successor(Node* node) const {
        if (node == nullptr) return nullptr;
        if (node->right != nullptr) {
            return minimum(node->right);
        }
        Node* p = node->parent;
        while (p != nullptr && node == p->right) {
            node = p;
            p = p->parent;
        }
        return p;
    }

    // Find predecessor of a node
    Node* predecessor(Node* node) const {
        if (node == nullptr) return nullptr;
        if (node->left != nullptr) {
            return maximum(node->left);
        }
        Node* p = node->parent;
        while (p != nullptr && node == p->left) {
            node = p;
            p = p->parent;
        }
        return p;
    }

    // Left rotate
    void left_rotate(Node* x) {
        Node* y = x->right;
        x->right = y->left;
        if (y->left != nullptr) {
            y->left->parent = x;
        }
        y->parent = x->parent;
        if (x->parent == nullptr) {
            root = y;
        } else if (x == x->parent->left) {
            x->parent->left = y;
        } else {
            x->parent->right = y;
        }
        y->left = x;
        x->parent = y;
    }

    // Right rotate
    void right_rotate(Node* y) {
        Node* x = y->left;
        y->left = x->right;
        if (x->right != nullptr) {
            x->right->parent = y;
        }
        x->parent = y->parent;
        if (y->parent == nullptr) {
            root = x;
        } else if (y == y->parent->right) {
            y->parent->right = x;
        } else {
            y->parent->left = x;
        }
        x->right = y;
        y->parent = x;
    }

    // Fix red-black tree after insertion
    void insert_fixup(Node* z) {
        while (z->parent != nullptr && z->parent->color == RED) {
            if (z->parent == z->parent->parent->left) {
                Node* y = z->parent->parent->right;
                if (y != nullptr && y->color == RED) {
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->right) {
                        z = z->parent;
                        left_rotate(z);
                    }
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    right_rotate(z->parent->parent);
                }
            } else {
                Node* y = z->parent->parent->left;
                if (y != nullptr && y->color == RED) {
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        right_rotate(z);
                    }
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    left_rotate(z->parent->parent);
                }
            }
        }
        root->color = BLACK;
    }

    // Transplant node u with node v
    void transplant(Node* u, Node* v) {
        if (u->parent == nullptr) {
            root = v;
        } else if (u == u->parent->left) {
            u->parent->left = v;
        } else {
            u->parent->right = v;
        }
        if (v != nullptr) {
            v->parent = u->parent;
        }
    }

    // Fix red-black tree after deletion
    void erase_fixup(Node* x, Node* x_parent) {
        while (x != root && (x == nullptr || x->color == BLACK)) {
            if (x == x_parent->left) {
                Node* w = x_parent->right;
                if (w != nullptr && w->color == RED) {
                    w->color = BLACK;
                    x_parent->color = RED;
                    left_rotate(x_parent);
                    w = x_parent->right;
                }
                if (w == nullptr || ((w->left == nullptr || w->left->color == BLACK) &&
                                     (w->right == nullptr || w->right->color == BLACK))) {
                    if (w != nullptr) w->color = RED;
                    x = x_parent;
                    x_parent = x->parent;
                } else {
                    if (w->right == nullptr || w->right->color == BLACK) {
                        if (w->left != nullptr) w->left->color = BLACK;
                        w->color = RED;
                        right_rotate(w);
                        w = x_parent->right;
                    }
                    w->color = x_parent->color;
                    x_parent->color = BLACK;
                    if (w->right != nullptr) w->right->color = BLACK;
                    left_rotate(x_parent);
                    x = root;
                }
            } else {
                Node* w = x_parent->left;
                if (w != nullptr && w->color == RED) {
                    w->color = BLACK;
                    x_parent->color = RED;
                    right_rotate(x_parent);
                    w = x_parent->left;
                }
                if (w == nullptr || ((w->right == nullptr || w->right->color == BLACK) &&
                                     (w->left == nullptr || w->left->color == BLACK))) {
                    if (w != nullptr) w->color = RED;
                    x = x_parent;
                    x_parent = x->parent;
                } else {
                    if (w->left == nullptr || w->left->color == BLACK) {
                        if (w->right != nullptr) w->right->color = BLACK;
                        w->color = RED;
                        left_rotate(w);
                        w = x_parent->left;
                    }
                    w->color = x_parent->color;
                    x_parent->color = BLACK;
                    if (w->left != nullptr) w->left->color = BLACK;
                    right_rotate(x_parent);
                    x = root;
                }
            }
        }
        if (x != nullptr) x->color = BLACK;
    }

    // Find node with given key
    Node* find_node(const Key& key) const {
        Node* current = root;
        while (current != nullptr) {
            if (comp(key, current->data->first)) {
                current = current->left;
            } else if (comp(current->data->first, key)) {
                current = current->right;
            } else {
                return current;
            }
        }
        return nullptr;
    }

public:
    /**
     * see BidirectionalIterator at CppReference for help.
     *
     * if there is anything wrong throw invalid_iterator.
     *     like it = map.begin(); --it;
     *       or it = map.end(); ++end();
     */
    class const_iterator;
    class iterator {
        friend class map;
        friend class const_iterator;
    private:
        const map* map_ptr;
        Node* node_ptr;

        iterator(const map* m, Node* n) : map_ptr(m), node_ptr(n) {}

    public:
        iterator() : map_ptr(nullptr), node_ptr(nullptr) {}

        iterator(const iterator& other) : map_ptr(other.map_ptr), node_ptr(other.node_ptr) {}

        /**
         * iter++
         */
        iterator operator++(int) {
            iterator temp = *this;
            ++(*this);
            return temp;
        }

        /**
         * ++iter
         */
        iterator& operator++() {
            if (node_ptr == nullptr) {
                throw invalid_iterator();
            }
            Node* next = map_ptr->successor(node_ptr);
            node_ptr = next;
            return *this;
        }

        /**
         * iter--
         */
        iterator operator--(int) {
            iterator temp = *this;
            --(*this);
            return temp;
        }

        /**
         * --iter
         */
        iterator& operator--() {
            if (node_ptr == nullptr) {
                // Decrementing end(), should give the maximum element
                if (map_ptr->root == nullptr) {
                    throw invalid_iterator();
                }
                node_ptr = map_ptr->maximum(map_ptr->root);
                return *this;
            }
            if (map_ptr->root != nullptr && node_ptr == map_ptr->minimum(map_ptr->root)) {
                throw invalid_iterator();
            }
            node_ptr = map_ptr->predecessor(node_ptr);
            if (node_ptr == nullptr) {
                throw invalid_iterator();
            }
            return *this;
        }

        /**
         * a operator to check whether two iterators are same (pointing to the same memory).
         */
        value_type& operator*() const {
            if (node_ptr == nullptr) {
                throw invalid_iterator();
            }
            return *(node_ptr->data);
        }

        bool operator==(const iterator& rhs) const {
            return map_ptr == rhs.map_ptr && node_ptr == rhs.node_ptr;
        }

        bool operator==(const const_iterator& rhs) const {
            return map_ptr == rhs.map_ptr && node_ptr == rhs.node_ptr;
        }

        /**
         * some other operator for iterator.
         */
        bool operator!=(const iterator& rhs) const {
            return !(*this == rhs);
        }

        bool operator!=(const const_iterator& rhs) const {
            return !(*this == rhs);
        }

        /**
         * for the support of it->first.
         * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
         */
        value_type* operator->() const noexcept {
            return node_ptr->data;
        }
    };

    class const_iterator {
        friend class map;
        friend class iterator;
    private:
        const map* map_ptr;
        Node* node_ptr;

        const_iterator(const map* m, Node* n) : map_ptr(m), node_ptr(n) {}

    public:
        const_iterator() : map_ptr(nullptr), node_ptr(nullptr) {}

        const_iterator(const const_iterator& other) : map_ptr(other.map_ptr), node_ptr(other.node_ptr) {}

        const_iterator(const iterator& other) : map_ptr(other.map_ptr), node_ptr(other.node_ptr) {}

        const_iterator operator++(int) {
            const_iterator temp = *this;
            ++(*this);
            return temp;
        }

        const_iterator& operator++() {
            if (node_ptr == nullptr) {
                throw invalid_iterator();
            }
            Node* next = map_ptr->successor(node_ptr);
            node_ptr = next;
            return *this;
        }

        const_iterator operator--(int) {
            const_iterator temp = *this;
            --(*this);
            return temp;
        }

        const_iterator& operator--() {
            if (node_ptr == nullptr) {
                // Decrementing end(), should give the maximum element
                if (map_ptr->root == nullptr) {
                    throw invalid_iterator();
                }
                node_ptr = map_ptr->maximum(map_ptr->root);
                return *this;
            }
            if (map_ptr->root != nullptr && node_ptr == map_ptr->minimum(map_ptr->root)) {
                throw invalid_iterator();
            }
            node_ptr = map_ptr->predecessor(node_ptr);
            if (node_ptr == nullptr) {
                throw invalid_iterator();
            }
            return *this;
        }

        const value_type& operator*() const {
            if (node_ptr == nullptr) {
                throw invalid_iterator();
            }
            return *(node_ptr->data);
        }

        bool operator==(const iterator& rhs) const {
            return map_ptr == rhs.map_ptr && node_ptr == rhs.node_ptr;
        }

        bool operator==(const const_iterator& rhs) const {
            return map_ptr == rhs.map_ptr && node_ptr == rhs.node_ptr;
        }

        bool operator!=(const iterator& rhs) const {
            return !(*this == rhs);
        }

        bool operator!=(const const_iterator& rhs) const {
            return !(*this == rhs);
        }

        const value_type* operator->() const noexcept {
            return node_ptr->data;
        }
    };

    /**
     * two constructors
     */
    map() : root(nullptr), node_count(0) {}

    map(const map& other) : root(nullptr), node_count(other.node_count), comp(other.comp) {
        root = copy_tree(other.root, nullptr);
    }

    /**
     * assignment operator
     */
    map& operator=(const map& other) {
        if (this != &other) {
            delete_tree(root);
            root = nullptr;
            node_count = other.node_count;
            comp = other.comp;
            root = copy_tree(other.root, nullptr);
        }
        return *this;
    }

    /**
     * Destructors
     */
    ~map() {
        delete_tree(root);
    }

    /**
     * access specified element with bounds checking
     * Returns a reference to the mapped value of the element with key equivalent to key.
     * If no such element exists, an exception of type `index_out_of_bound'
     */
    T& at(const Key& key) {
        Node* node = find_node(key);
        if (node == nullptr) {
            throw index_out_of_bound();
        }
        return node->data->second;
    }

    const T& at(const Key& key) const {
        Node* node = find_node(key);
        if (node == nullptr) {
            throw index_out_of_bound();
        }
        return node->data->second;
    }

    /**
     * access specified element
     * Returns a reference to the value that is mapped to a key equivalent to key,
     *   performing an insertion if such key does not already exist.
     */
    T& operator[](const Key& key) {
        Node* node = find_node(key);
        if (node != nullptr) {
            return node->data->second;
        }
        // Insert new node with default-constructed value
        auto result = insert(value_type(key, T()));
        return result.first.node_ptr->data->second;
    }

    /**
     * behave like at() throw index_out_of_bound if such key does not exist.
     */
    const T& operator[](const Key& key) const {
        return at(key);
    }

    /**
     * return a iterator to the beginning
     */
    iterator begin() {
        if (root == nullptr) {
            return iterator(this, nullptr);
        }
        return iterator(this, minimum(root));
    }

    const_iterator cbegin() const {
        if (root == nullptr) {
            return const_iterator(this, nullptr);
        }
        return const_iterator(this, minimum(root));
    }

    /**
     * return a iterator to the end
     * in fact, it returns past-the-end.
     */
    iterator end() {
        return iterator(this, nullptr);
    }

    const_iterator cend() const {
        return const_iterator(this, nullptr);
    }

    /**
     * checks whether the container is empty
     * return true if empty, otherwise false.
     */
    bool empty() const {
        return node_count == 0;
    }

    /**
     * returns the number of elements.
     */
    size_t size() const {
        return node_count;
    }

    /**
     * clears the contents
     */
    void clear() {
        delete_tree(root);
        root = nullptr;
        node_count = 0;
    }

    /**
     * insert an element.
     * return a pair, the first of the pair is
     *   the iterator to the new element (or the element that prevented the insertion),
     *   the second one is true if insert successfully, or false.
     */
    pair<iterator, bool> insert(const value_type& value) {
        Node* parent = nullptr;
        Node* current = root;

        while (current != nullptr) {
            parent = current;
            if (comp(value.first, current->data->first)) {
                current = current->left;
            } else if (comp(current->data->first, value.first)) {
                current = current->right;
            } else {
                return pair<iterator, bool>(iterator(this, current), false);
            }
        }

        Node* new_node = new Node(value, parent);
        node_count++;

        if (parent == nullptr) {
            root = new_node;
        } else if (comp(value.first, parent->data->first)) {
            parent->left = new_node;
        } else {
            parent->right = new_node;
        }

        insert_fixup(new_node);
        return pair<iterator, bool>(iterator(this, new_node), true);
    }

    /**
     * erase the element at pos.
     *
     * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
     */
    void erase(iterator pos) {
        if (pos.map_ptr != this || pos.node_ptr == nullptr) {
            throw invalid_iterator();
        }

        Node* z = pos.node_ptr;
        Node* y = z;
        Node* x;
        Node* x_parent;
        Color y_original_color = y->color;

        if (z->left == nullptr) {
            x = z->right;
            x_parent = z->parent;
            transplant(z, z->right);
        } else if (z->right == nullptr) {
            x = z->left;
            x_parent = z->parent;
            transplant(z, z->left);
        } else {
            y = minimum(z->right);
            y_original_color = y->color;
            x = y->right;
            if (y->parent == z) {
                x_parent = y;
            } else {
                x_parent = y->parent;
                transplant(y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }
            transplant(z, y);
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color;
        }

        delete z;
        node_count--;

        if (y_original_color == BLACK) {
            erase_fixup(x, x_parent);
        }
    }

    /**
     * Returns the number of elements with key
     *   that compares equivalent to the specified argument,
     *   which is either 1 or 0
     *     since this container does not allow duplicates.
     * The default method of check the equivalence is !(a < b || b > a)
     */
    size_t count(const Key& key) const {
        return find_node(key) != nullptr ? 1 : 0;
    }

    /**
     * Finds an element with key equivalent to key.
     * key value of the element to search for.
     * Iterator to an element with key equivalent to key.
     *   If no such element is found, past-the-end (see end()) iterator is returned.
     */
    iterator find(const Key& key) {
        Node* node = find_node(key);
        if (node == nullptr) {
            return end();
        }
        return iterator(this, node);
    }

    const_iterator find(const Key& key) const {
        Node* node = find_node(key);
        if (node == nullptr) {
            return cend();
        }
        return const_iterator(this, node);
    }
};

}

#endif
