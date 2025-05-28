#include <iostream>
#include <string>
#include <assert.h>
#include <iomanip>
#include <vector>

using namespace std;

using Key = string; // 구현 단순화(템플릿 미사용)
using Value = int;  // 구현 단순화(템플릿 미사용)

enum Color { kRed, kBlack };

class Node
{
public:
	Key key;
	Value val;
	Node* left;
	Node* right;
	Color color;

	Node(Key key, Value val, int N, Color color)
		: key(key), val(val), color(color),
		left(nullptr), right(nullptr) // 널 포인터로 초기화
	{
	}

	// 자바로 구현할 때는 널포인터 대신에 
	// 가상의 널 오브젝트를 하나 만들어서 가리키도록 구현할 수 있습니다.
	// C++에서 굳이 그렇게 구현할 필요가 없기 때문에
	// 여기서는 자식이 없으면 널포인터(nullptr)를 대입해놓습니다.
};

class RedBlackBST
{
public:
	Node* root = nullptr;

	bool IsEmpty() { return root == nullptr; }

	bool IsRed(Node* x)
	{
		if (x == nullptr) return false; // 널노드는 블랙
		return x->color == Color::kRed;
	}

	// 이진트리 복습
	Value Search(Key key) { return Search(root, key); }
	Value Search(Node* x, Key key)
	{
		if (x == nullptr)
			return -1; // 편의상 못 찾았을 경우 -1 반환

		if (key < x->key)
			return Search(x->left, key);
		else if (key > x->key)
			return Search(x->right, key);
		else
			return x->val;
	}

	// 이진트리 복습
	bool Contains(Key key) { return Contains(root, key); }
	bool Contains(Node* x, Key key)
	{
		if (x == nullptr)
			return false;

		if (key < x->key)
			return Contains(x->left, key);
		else if (key > x->key)
			return Contains(x->right, key);
		else
			return true;

		return false;
	}

	// 키(key)가 가장 작은 노드 찾기 (이진트리 복습)
	Key Min()
	{
		assert(!IsEmpty());
		return Min(root)->key;
	}

	Node* Min(Node* x)
	{
		Node* y = nullptr;

		return (x->left == nullptr) ? x : Min(x->left);
	}

	Key Max()
	{
		assert(!IsEmpty());
		return Max(root)->key;
	}
	Node* Max(Node* x)
	{
		Node* y = nullptr;

		return (x->right == nullptr) ? x : Max(x->right);
	}

	Node* RotateLeft(Node* h)
	{
		Node* x = h->right;
		h->right = x->left;
		x->left = h;
		x->color = h->color;
		h->color = Color::kRed;

		return x;
	}

	Node* RotateRight(Node* h)
	{
		Node* x = h->left;
		h->left = x->right;
		x->right = h;
		x->color = h->color;
		h->color = Color::kRed;

		return x;
	}

	void FlipColor(Color& c)
	{
		if (c == Color::kRed)
			c = Color::kBlack;
		else
			c = Color::kRed;
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

	void Insert(Key key, Value val)
	{
		root = Insert(root, key, val);
		root->color = Color::kBlack; // 루트는 블랙
	}

	Node* Insert(Node* h, Key key, Value val)
	{
		if (h == nullptr)
			return new Node(key, val, 1, Color::kRed);

		// key를 비교해서 재귀호출 (키가 같으면 val 업데이트)
		// 이진트리와 동일
		if (key < h->key)
			h->left = Insert(h->left, key, val);
		else if (key > h->key)
			h->right = Insert(h->right, key, val);
		else
			h->val = val;

		// Insert()는 재귀호출됩니다.
		// 여기서 반환되면 트리를 거슬러 올라가면서 
		// 추가로 자식들의 색을 보고 수정합니다.

		return Balance(h); // <- Balance() 구현
	}

	// 삭제할 때 사용됨 (실행 예시 설명 참고)
	Node* MoveRedLeft(Node* h)
	{
		cout << "MoveRedLeft() " << h->key << endl;

		FlipColors(h);

		if (IsRed(h->right->left))
		{
			h->right = RotateRight(h->right);
			h = RotateLeft(h);
			FlipColors(h);
		}

		return h;
	}

	// 삭제할 때 사용됨 (MoveRedLeft와 좌우대칭)
	Node* MoveRedRight(Node* h)
	{
		cout << "MoveRedRight() " << h->key << endl;

		FlipColors(h);

		if (IsRed(h->left->left))
		{
			h = RotateRight(h);
			FlipColors(h);
		}

		return h;
	}

	// 가장 작은 키(key)를 찾아서 삭제
	void DeleteMin()
	{
		assert(!IsEmpty());

		// 루트가 가운데인 4-노드로 임시 변경
		if (!IsRed(root->left) && !IsRed(root->right))
			root->color = Color::kRed;

		root = DeleteMin(root);

		// 루트는 항상 블랙
		if (!IsEmpty())
			root->color = Color::kBlack;
	}


	Node* DeleteMin(Node* h)
	{
		cout << "DeleteMin() " << h->key << endl;

		// h가 가장 작은 노드라면 삭제하고 반환
		if (h->left == nullptr)
		{
			cout << "Delete node " << h->key << endl;
			delete h; // 자바에서는 가비지 컬렉터 사용
			return nullptr;
		}

		// 왼쪽자식이 블랙이고, 왼쪽-왼쪽 자식도 블랙이면
		// 삭제하기 어렵기 때문에 MoveRedLeft()에서 3-노드로 변경
		if (!IsRed(h->left) && !IsRed(h->left->left))
		{
			h = MoveRedLeft(h);
			Print2D(h);
		}

		// 계속 찾아 내려감
		h->left = DeleteMin(h->left);

		return Balance(h);
	}

	// 가장 큰 키(key)를 찾아서 삭제 (DeleteMin과 대칭)
	void DeleteMax()
	{
		if (!IsRed(root->left) && !IsRed(root->right))
			root->color = Color::kRed;;
		root = DeleteMax(root);
		if (!IsEmpty()) root->color = Color::kBlack;
	}

	Node* DeleteMax(Node* h)
	{
		// 실행 예시에서 *S-X에서 X 삭제 참고
		// *S가 X의 왼쪽 자식이기 때문에 *S를 위로 올려서
		// X가 *S의 오른쪽 자식이 되도록 만든 후에 X 삭제
		if (IsRed(h->left))
			h = RotateRight(h);

		if (h->right == nullptr)
		{
			delete h; // 자바는 가비지 컬렉터 사용
			return nullptr;
		}

		if (!IsRed(h->right) && !IsRed(h->right->left))
			h = MoveRedRight(h);

		h->right = DeleteMax(h->right);

		return Balance(h);
	}

	// 임의의 키(key)를 찾아서 삭제
	void Delete(Key key)
	{
		// 삭제하려는 키를 가진 노드가 존재하는지 미리 확인
		if (!Contains(key)) return;

		if (!IsRed(root->left) && !IsRed(root->right))
			root->color = Color::kRed;

		root = Delete(root, key);

		if (!IsEmpty()) root->color = Color::kBlack;
	}

	Node* Delete(Node* h, Key key)
	{
		if (key < h->key) // 왼쪽으로 찾아 내려가서 지우는 경우
		{
			// DeleteMin()과 비슷함

			if (!IsRed(h->left) && !IsRed(h->left->left))
				h = MoveRedLeft(h);

			h->left = Delete(h->left, key);
		}
		else // 오른쪽으로 찾아 내려가거나 바로 삭제하는 경우
		{
			// DeleteMax()와 비슷한 경우
			if (IsRed(h->left))
				h = RotateRight(h);

			// 키가 일치하고 오른쪽 서브트리가 없으면 삭제
			// 왼쪽 서브트리에 대한 처리는 바로 위의 RotateRight()에 해줬음
			if ((key == h->key) && (h->right == nullptr))
			{
				delete h; // 자바는 가비지 컬렉터 사용
				return nullptr;
			}

			if (!IsRed(h->right) && !IsRed(h->right->left))
				h = MoveRedRight(h);

			// 삭제하는 경우
			if (key == h->key)
			{
				// 오른쪽 서브트리에서 가장 작은 것을 h로 복사한 후에
				// DeleteMin()으로 그것을 삭제
				Node* x = Min(h->right);
				h->key = x->key;
				h->val = x->val;
				h->right = DeleteMin(h->right);
			}
			else {
				// 오른쪽으로 계속 찾아가기
				h->right = Delete(h->right, key);
			}
		}

		return Balance(h);
	}

	// 루트만 있을 경우 높이 0
	// https://en.wikipedia.org/wiki/Tree_(data_structure)
	int Height() { return Height(root); }
	int Height(Node* node)
	{
		if (!node) return -1;
		return 1 + std::max(Height(node->left), Height(node->right));
	}

	// 디버깅 편의 도구
	vector<string> screen;
	void PrintLine(int x, string s, string& line) {
		for (const auto c : s) line[x++] = c;
		//cout << line << endl;
	}
	void Print2D() { Print2D(root); }
	void Print2D(Node* root) {
		if (!root) cout << "Empty" << endl;
		else {
			int h = Height(root) + 1, w = 4 * int(pow(2, h - 1));
			screen.clear();
			screen.resize(h * 2, string(w, ' '));
			Print2D(root, w / 2 - 2, 0, h - 1);
			for (const auto& l : screen) cout << l << endl;
		}
	}
	void Print2D(Node* n, int x, int level, int s)
	{
		//cout << x << " " << level << " " << s << endl;
		PrintLine(x, (IsRed(n) ? "*" : " ") + n->key, screen[2 * level]);
		x -= int(pow(2, s));
		if (n->left) {
			PrintLine(x, "  /", screen[2 * level + 1]);
			Print2D(n->left, x, level + 1, s - 1);
		}
		if (n->right)
		{
			PrintLine(x + 2 * int(pow(2, s)), "\\", screen[2 * level + 1]);
			Print2D(n->right, x + 2 * int(pow(2, s)), level + 1, s - 1);
		}
	}
};
