// Copyright 2021 William Bowers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Slave class with the latest data living on the server
// We are just treating this like a struct
class AimProfile {
	int lastUpdated = -1;
	vector out_front, out_back;
	float Azmuith;
	void AimProfile(vector out_f, vector out_b) {
		out_front = out_f;
		out_back = out_b;
		lastUpdated = GetGame().GetTime();
	}
};

// Class managing the list of weapon world space data
class eAIServerAimProfileManager {
	//autoptr map<Weapon_Base, ref AimProfile> weapons = new map<Weapon_Base, ref AimProfile>();
	
	// todo map cleanup
	
	void eAIAimDetails(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param3<Weapon_Base, vector, vector> data;
		if (!ctx.Read(data)) return;
		/*AimProfile a = weapons.Get(data.param1);
		if (a) { // If we already have an eentry for this weapon
			a.lastUpdated = GetGame().GetTime();
			a.out_front = data.param2;
			a.out_back = data.param3;
		} else { // otherwise
			weapons.Insert(data.param1, new AimProfile(data.param2, data.param3));
		}*/
		
		// Add an additional temporary way to lookup the two positions
		if (data.param1 && data.param1.aim) {
			data.param1.aim.out_front = data.param2;
			data.param1.aim.out_back = data.param3;
			data.param1.aim.lastUpdated = GetGame().GetTime();
			data.param1.aim.Azmuith = (data.param2 - data.param3).VectorToAngles().GetRelAngles()[0];
		} else {
			Print("eAIAimDetails updated, but no weapon exists! At: " + data.param2.ToString());
		}
	}
};