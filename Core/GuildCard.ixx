export module Core.GuildCard;

import Models.GuildCard;
import Core.Card;

export namespace Core
{
	export class GuildCard:public Core::Card
	{
		public:
			Models::GuildCard m_guildCard;
	private:
		bool isVisible = false;
		bool isAccessible = false;
	public:
		const bool& getCardVisible() const;
		void setCardVisibility(const bool& visible);
		const bool& getCardAccessible() const;
		void setCardAccessibility(const bool& accessible);
	};
}