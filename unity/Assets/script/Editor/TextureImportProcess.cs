using UnityEngine;
using UnityEditor;
using System.IO;

public class TextureImportProcess : AssetPostprocessor {
	void OnPreprocessTexture () {
		var importer = assetImporter as TextureImporter;
        var path = importer.assetPath;

		importer.isReadable = true;
		importer.textureCompression = TextureImporterCompression.Uncompressed;
	}
}