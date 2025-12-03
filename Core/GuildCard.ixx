export module Core.GuildCard;

import Models.GuildCard;


export namespace Core
{
	export class GuildCard
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