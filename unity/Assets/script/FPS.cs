/*
* Viry3D
* Copyright 2014-2018 by Stack - stackos@qq.com
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

using UnityEngine;
using UnityEngine.UI;

public class FPS : MonoBehaviour {
	int frame_count = 0;
	float time = 0;
	int fps = 0;
	Text label;

	// Use this for initialization
	void Start () {
		label = GetComponent<Text>();
	}
	
	// Update is called once per frame
	void Update () {
		var t = Time.realtimeSinceStartup;
		if (t - time >= 1) {
			time = t;
			fps = frame_count;
			frame_count = 0;

			label.text = "fps:" + fps;
		}

		frame_count++;
	}
}
