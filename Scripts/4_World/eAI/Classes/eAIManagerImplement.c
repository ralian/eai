enum eAIProcessingState
{
	UNPROCESSED = 0,
	PROCESSED,
	REMOVE
};

class eAIManagerImplement extends eAIManagerBase
{
	override void OnUpdate(bool doSim, float timeslice)
    {
		super.OnUpdate(doSim, timeslice);

		eAIGroup.UpdateAll(timeslice);

		if (GetGame().IsClient() || !GetGame().IsMultiplayer())
		{
			DayZPlayerImplement player;
			Class.CastTo(player, GetGame().GetPlayer());

			SetInGroup(player && player.GetGroup() != null);
		}
	}

	/*
	// List of all eAI entities
	private autoptr array<eAIProcessingState> m_AIStates = {};
	private autoptr array<ref eAIPlayerHandler> m_AI = {};

    private int m_ProcessingIndex = 0;
	private int m_MaxProcessingAI = 0;
	private float m_MinimumTime = 0.5; // 0.050; // 20hz
	private float m_ProcessingTime = 0;

    override Class AddAI(DayZPlayer entity)
    {
        eAIPlayerHandler handler = new eAIPlayerHandler(PlayerBase.Cast(entity));
        m_AI.Insert(handler); // insert the new handler to the back of the array
        return handler;
    }
	
	override void OnUpdate(bool doSim, float timeslice)
    {
        // don't process if we aren't the server
        if (!GetGame().IsServer()) return;

		m_ProcessingTime += timeslice;
		
		// we are back at the start
		if (m_ProcessingIndex == 0)
		{
			// we have processed all the AI within the minimum time, we can wait.
			if (m_ProcessingTime < m_MinimumTime)
			{
				return;
			}
			
			m_ProcessingTime = 0;

			if (m_AIStates.Count() == m_MaxProcessingAI)
			{
				// remove all AI that was marked for removal
				for (int i = m_AIStates.Count() - 1; i >= 0; i--)
				{
					switch (m_AIStates[i])
					{
						case eAIProcessingState.REMOVE:
						{
							m_AI.RemoveOrdered(i);
							break;
						}
					}
				}
			} else
			{
				Error("An unexpected desync with AI processing happened.");
			}

			// how many AI we can process for this next simulation step
			m_MaxProcessingAI = m_AI.Count();
			m_AIStates.Clear();

			// if we have 0 ai to process, wait 2 "ai" frames
			if (m_MaxProcessingAI == 0)
			{
				m_ProcessingTime = -m_MinimumTime;
				return;
			}
		}

		if (m_ProcessingIndex < m_MaxProcessingAI)
		{
			// this is the AI we will be processing this frame
			eAIPlayerHandler ai_Handler = m_AI[m_ProcessingIndex];
			m_ProcessingIndex++;

			// ai is null
			if (ai_Handler == null)
			{
				m_AIStates.Insert(eAIProcessingState.REMOVE);
				return;
			}

			// is dead
			if (ai_Handler.isDead())
			{
				m_AIStates.Insert(eAIProcessingState.REMOVE);
				return;
			}

			// update the AI
			ai_Handler.OnTick();

			// mark the AI as processed
			m_AIStates.Insert(eAIProcessingState.PROCESSED);
		} 
		else 
		{
			// reset the AI index if all are processed
			m_ProcessingIndex = 0; 
		}
	}
	*/
};