export module Core.AgeTree;
import Core.Node;
import Models.Card;
import <vector>;
import <memory>;
export namespace Core {
	class Age1Tree {
	private:
		std::vector<std::shared_ptr<Node>> m_nodes;
	public:
		Age1Tree(std::vector<std::unique_ptr<Models::Card>>&& cards);
		std::vector<std::shared_ptr<Node>> releaseNodes();
	};
	class Age2Tree {
	private:
		std::vector<std::shared_ptr<Node>> m_nodes;
	public:
		Age2Tree(std::vector<std::unique_ptr<Models::Card>>&& cards);
		std::vector<std::shared_ptr<Node>> releaseNodes();
	};
	class Age3Tree {
	private:
		std::vector<std::shared_ptr<Node>> m_nodes;
	public:
		Age3Tree(std::vector<std::unique_ptr<Models::Card>>&& cards);
		std::vector<std::shared_ptr<Node>> releaseNodes();
	};
}
