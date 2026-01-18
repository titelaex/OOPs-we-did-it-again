export module Models.ICard;

import Models.Age;

export namespace Models
{
	export class ICard
	{
	public:
		virtual ~ICard() = default;
		/*virtual void toggleVisibility() = 0;
		virtual void toggleAccessibility() = 0;*/
	};
}
