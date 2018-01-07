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
using UnityEditor;
using System.Collections.Generic;
using System.IO;

public class SpriteSlice : AssetPostprocessor {
    void OnPreprocessTexture () {
        if(assetPath.Equals("Assets/AppFlappyBird/atlas.png")) {
            var importer = (TextureImporter) assetImporter;
            importer.textureType = TextureImporterType.Sprite;
            importer.spriteImportMode = SpriteImportMode.Multiple;

            List<SpriteMetaData> metas = new List<SpriteMetaData>();

            texture = AssetDatabase.LoadAssetAtPath<Texture2D>("Assets/AppFlappyBird/atlas.png");
            atlasInfo = AssetDatabase.LoadAssetAtPath<TextAsset>("Assets/AppFlappyBird/atlas.txt");
            var lines = atlasInfo.text.Split(new string[] { "\n" }, System.StringSplitOptions.RemoveEmptyEntries);
		    foreach(var i in lines) {
			    var values = i.Split(new string[] { " " }, System.StringSplitOptions.RemoveEmptyEntries);
			    if(values.Length == 7) {
				    var name = values[0];
				    var pw = System.Convert.ToInt32(values[1]);
				    var ph = System.Convert.ToInt32(values[2]);
				    var x = System.Convert.ToSingle(values[3]);
				    var y = System.Convert.ToSingle(values[4]);
				    var w = System.Convert.ToSingle(values[5]);
				    var h = System.Convert.ToSingle(values[6]);

                    var m = new SpriteMetaData();
                    m.alignment = (int) SpriteAlignment.Center;
                    m.border = Vector4.zero;
                    m.name = name;
                    m.pivot = new Vector2(0.5f, 0.5f);
                    m.rect = new Rect(texture.width * x, texture.height * (1 - y) - ph, pw, ph);

                    metas.Add(m);
			    }
		    }

            importer.spritesheet = metas.ToArray();
            importer.spritePixelsPerUnit = 100;
        }
    }

	public Texture2D texture;
	public TextAsset atlasInfo;

	void Start () {
		var path = AssetDatabase.GetAssetPath (texture);

		var spriteDir = new FileInfo(path).DirectoryName + "/" + texture.name;
		if(!Directory.Exists(spriteDir)) {
			Directory.CreateDirectory (spriteDir);
		}

		var lines = atlasInfo.text.Split (new string[] { "\n" }, System.StringSplitOptions.RemoveEmptyEntries);
		foreach (var i in lines) {
			var values = i.Split (new string[] { " " }, System.StringSplitOptions.RemoveEmptyEntries);
			if(values.Length == 7) {
				var name = values[0];
				var pw = System.Convert.ToInt32 (values[1]);
				var ph = System.Convert.ToInt32 (values[2]);
				var x = System.Convert.ToSingle (values[3]);
				var y = System.Convert.ToSingle (values[4]);
				var w = System.Convert.ToSingle (values[5]);
				var h = System.Convert.ToSingle (values[6]);

				var s = Sprite.Create(texture, new Rect(texture.width * x, texture.height * (1 - y) - ph, pw, ph), new Vector2(0.5f, 0.5f), 100, 0, SpriteMeshType.FullRect, Vector4.zero);
				AssetDatabase.CreateAsset (s, "Assets" + spriteDir.Substring(Application.dataPath.Length) + "/" + name + ".asset");
			}
		}
	}
}