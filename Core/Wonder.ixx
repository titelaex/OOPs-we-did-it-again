export module Core.Wonder;

import Models.Wonder;

export namespace Core
{
	export class Wonder {
	public:
		Models::Wonder m_wonder;
	private:
		bool isConstructed = false;
	public:
		const bool& getIsConstructed() const;
		void setIsConstructed(const bool& constructed);
	};
}