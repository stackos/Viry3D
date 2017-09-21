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
