modded class PluginAdminLog {
	override void PlayerHitBy( TotalDamageResult damageResult, int damageType, PlayerBase player, EntityAI source, int component, string dmgZone, string ammo ) {
		if (player.IsAI()) return; // We have to do this for now since they don't have an identity
		super.PlayerHitBy(damageResult, damageType, player, source, component, dmgZone, ammo);
	}
	
	override void PlayerKilled( PlayerBase player, Object source ) {
		if (player.IsAI()) return; // We have to do this for now since they don't have an identity
		super.PlayerKilled(player, source);
	}
};