export module Models.ICard;

import Models.Age;

export namespace Models
{
	export class ICard
	{
	public:
		virtual ~ICard() = default;
		virtual void toggleVisibility() = 0;
		virtual void displayCardInfo() = 0;
		virtual void setAge(Age age) = 0;
	};
}
