#include <iostream>
#include <string>
#include <assert.h>
#include <iomanip>
#include <vector>

using namespace std;

using Key = string;
using Value = int;

enum Color
{
	RED,
	BLACK
};

class Node
{
public:
	Key		key;
	Value	value;
	Node*	left;
	Node*	right;
	Color	color;

	Node(Key k, Value v, Color c) 
		: key(k), value(v), left(nullptr), right(nullptr), color(c) {}
};

class RedBlackTree
{
public:
	Node* root = nullptr;

	bool IsEmpty() { return root = nullptr; }

	bool IsRed(Node* node)
	{
		if (node == nullptr)
			return false;

		return node->color == Color::RED;
	}

	Value Search(Key key) { return Search(root, key); }
	Value Search(Node* node, Key key)
	{
		if (node == nullptr)
		{
			std::cout << "node is nullptr\n";
			return INT_MIN;
		}

		if (key < node->key)
			return Search(node->left, key);
		else if (key > node->key)
			return Search(node->right, key);
		else
			return node->value;
	}

	bool Contains(Key key) { return Contains(root, key); }
	bool Contains(Node* node, Key key)
	{
		if (node == nullptr)
		{
			std::cout << "node is nullptr\n";
			return false;
		}

		if (key < node->key)
			return Contains(node->left, key);
		else if (key > node->key)
			return Contains(node->right, key);
		else
			return true;
	}

	Key Min()
	{
		assert(!IsEmpty());
		return Min(root)->key;
	}
	Node* Min(Node* node)
	{
		Node* min = nullptr;
		while (node != nullptr)
		{
			min = node;
			node = node->left;
		}

		return min;
	}

	Key Max()
	{
		assert(!IsEmpty());
		return Max(root)->key;
	}
	Node* Max(Node* node)
	{
		Node* max = nullptr;
		while (node != nullptr)
		{
			max = node;
			node = node->right;
		}

		return max;
	}

	Node* RotateLeft(Node* h)
	{
		Node* x = h->right;
		h->right = x->left;
		x->left = h;
		x->color = h->color;
		h->color = Color::RED;

		return x;
	}

	Node* RotateRight(Node* h)
	{
		Node* x = h->left;
		h->left = x->right;
		x->right = h;
		x->color = h->color;
		h->color = Color::RED;

		return x;
	}

	void FlipColor(Color& c)
	{
		if (c == Color::RED)
			c == Color::BLACK;
		else
			c == Color::RED;
	}

	void FlipColors(Node* h)
	{
		FlipColor(h->color);
		FlipColor(h->left->color);
		FlipColor(h->right->color);
	}

	Node* Balance(Node* h) 	// restore red-black tree invariant
	{
		assert(h != nullptr);

		if (IsRed(h->right) && !IsRed(h->left))
			h = RotateLeft(h);

		if (IsRed(h->left) && IsRed(h->left->left))
			h = RotateRight(h);

		if (IsRed(h->left) && IsRed(h->right))
			FlipColors(h);

		return h;
	}

};
