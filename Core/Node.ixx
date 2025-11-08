export module Node;

import Card;

export namespace Core {
	class Node {
	private:

	public:
		Node() = default;
		virtual ~Node() = default;
		virtual void display() const = 0;
	};
} 