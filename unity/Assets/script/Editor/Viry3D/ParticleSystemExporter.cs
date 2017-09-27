using UnityEngine;

public partial class Exporter {
	static void WriteParticleSystemRenderer(ParticleSystemRenderer renderer) {
		m_writer.Write(renderer.enabled);
		if(!renderer.enabled) {
			return;
		}

		m_writer.Write((int) renderer.renderMode);

		if(renderer.renderMode == ParticleSystemRenderMode.Stretch) {
			m_writer.Write(renderer.cameraVelocityScale);
			m_writer.Write(renderer.velocityScale);
			m_writer.Write(renderer.lengthScale);
		} else if(renderer.renderMode == ParticleSystemRenderMode.Mesh) {
			var mesh = renderer.mesh;
			if(mesh != null) {
				WriteMesh(mesh);
			} else {
				WriteString("");
			}
		}

		m_writer.Write(renderer.normalDirection);
		m_writer.Write((int) renderer.sortMode);
		m_writer.Write(renderer.sortingFudge);
		m_writer.Write(renderer.minParticleSize);
		m_writer.Write(renderer.maxParticleSize);
		m_writer.Write((int) renderer.alignment);
		WriteVector3(renderer.pivot);

		WriteRendererMaterials(renderer);
	}

	static void WriteParticleSystem(ParticleSystem ps) {
		WriteParticleSystemMain(ps.main);
		WriteParticleSystemEmission(ps.emission);
		WriteParticleSystemShape(ps.shape);
		WriteParticleSystemVelocityOverLifetime(ps.velocityOverLifetime);
		WriteParticleSystemLimitVelocityOverLifetime(ps.limitVelocityOverLifetime);
		WriteParticleSystemInheritVelocity(ps.inheritVelocity);
		WriteParticleSystemForceOverLifetime(ps.forceOverLifetime);
		WriteParticleSystemColorOverLifetime(ps.colorOverLifetime);
		WriteParticleSystemColorBySpeed(ps.colorBySpeed);
		WriteParticleSystemSizeOverLifetime(ps.sizeOverLifetime);
		WriteParticleSystemSizeBySpeed(ps.sizeBySpeed);
		WriteParticleSystemRotationOverLifetime(ps.rotationOverLifetime);
		WriteParticleSystemRotationBySpeed(ps.rotationBySpeed);
		WriteParticleSystemExternalForces(ps.externalForces);
		WriteParticleSystemTextureSheetAnimation(ps.textureSheetAnimation);
	}

	static void WriteParticleSystemMain(ParticleSystem.MainModule module) {
		m_writer.Write(module.duration);
		m_writer.Write(module.loop);

		WriteMinMaxCurve(module.startDelay);
		WriteMinMaxCurve(module.startLifetime);
		WriteMinMaxCurve(module.startSpeed);

		m_writer.Write(module.startSize3D);
		if(module.startSize3D) {
			WriteMinMaxCurve(module.startSizeX);
			WriteMinMaxCurve(module.startSizeY);
			WriteMinMaxCurve(module.startSizeZ);
		} else {
			WriteMinMaxCurve(module.startSize);
		}

		m_writer.Write(module.startRotation3D);
		if(module.startRotation3D) {
			WriteMinMaxCurve(module.startRotationX);
			WriteMinMaxCurve(module.startRotationY);
			WriteMinMaxCurve(module.startRotationZ);
		} else {
			WriteMinMaxCurve(module.startRotation);
		}

		m_writer.Write(module.randomizeRotationDirection);

		WriteMinMaxGradient(module.startColor);
		WriteMinMaxCurve(module.gravityModifier);
		m_writer.Write((int) module.simulationSpace);
		m_writer.Write(module.simulationSpeed);
		m_writer.Write((int) module.scalingMode);
		m_writer.Write(module.maxParticles);
	}

	static void WriteParticleSystemEmission(ParticleSystem.EmissionModule module) {
		m_writer.Write(module.enabled);
		if(!module.enabled) {
			return;
		}

		WriteMinMaxCurve(module.rateOverTime);
		WriteMinMaxCurve(module.rateOverDistance);

		m_writer.Write(module.burstCount);
		ParticleSystem.Burst[] bursts = new ParticleSystem.Burst[module.burstCount];
		module.GetBursts(bursts);
		for(int i = 0; i < bursts.Length; i++) {
			m_writer.Write(bursts[i].time);
			m_writer.Write(bursts[i].minCount);
			m_writer.Write(bursts[i].maxCount);
			m_writer.Write(bursts[i].cycleCount);
			m_writer.Write(bursts[i].repeatInterval);
		}
	}

	static void WriteParticleSystemShape(ParticleSystem.ShapeModule module) {
		m_writer.Write(module.enabled);
		if(!module.enabled) {
			return;
		}

		m_writer.Write((int) module.shapeType);
		if(module.shapeType == ParticleSystemShapeType.Sphere ||
			module.shapeType == ParticleSystemShapeType.SphereShell ||
			module.shapeType == ParticleSystemShapeType.Hemisphere ||
			module.shapeType == ParticleSystemShapeType.HemisphereShell) {
			m_writer.Write(module.radius);
		} else if(module.shapeType == ParticleSystemShapeType.Cone ||
			module.shapeType == ParticleSystemShapeType.ConeShell ||
			module.shapeType == ParticleSystemShapeType.ConeVolume ||
			module.shapeType == ParticleSystemShapeType.ConeVolumeShell) {
			m_writer.Write(module.angle);
			m_writer.Write(module.radius);
			m_writer.Write(module.arc);
			m_writer.Write((int) module.arcMode);
			m_writer.Write(module.arcSpread);
			WriteMinMaxCurve(module.arcSpeed);
			m_writer.Write(module.length);
		} else if(module.shapeType == ParticleSystemShapeType.Box ||
			module.shapeType == ParticleSystemShapeType.BoxShell ||
			module.shapeType == ParticleSystemShapeType.BoxEdge) {
			WriteVector3(module.scale);
		} else if(module.shapeType == ParticleSystemShapeType.Circle ||
			module.shapeType == ParticleSystemShapeType.CircleEdge) {
			m_writer.Write(module.radius);
			m_writer.Write(module.arc);
			m_writer.Write((int) module.arcMode);
			m_writer.Write(module.arcSpread);
			WriteMinMaxCurve(module.arcSpeed);
		} else if(module.shapeType == ParticleSystemShapeType.SingleSidedEdge) {
			m_writer.Write(module.radius);
			m_writer.Write((int) module.radiusMode);
			m_writer.Write(module.radiusSpread);
			WriteMinMaxCurve(module.radiusSpeed);
		}

		m_writer.Write(module.alignToDirection);
		m_writer.Write(module.randomDirectionAmount);
		m_writer.Write(module.sphericalDirectionAmount);
	}

	static void WriteParticleSystemVelocityOverLifetime(ParticleSystem.VelocityOverLifetimeModule module) {
		m_writer.Write(module.enabled);
		if(!module.enabled) {
			return;
		}

		WriteMinMaxCurve(module.x);
		WriteMinMaxCurve(module.y);
		WriteMinMaxCurve(module.z);
		m_writer.Write((int) module.space);
	}

	static void WriteParticleSystemLimitVelocityOverLifetime(ParticleSystem.LimitVelocityOverLifetimeModule module) {
		m_writer.Write(module.enabled);
		if(!module.enabled) {
			return;
		}

		m_writer.Write(module.separateAxes);
		if(module.separateAxes) {
			WriteMinMaxCurve(module.limitX);
			WriteMinMaxCurve(module.limitY);
			WriteMinMaxCurve(module.limitZ);
			m_writer.Write((int) module.space);
		} else {
			WriteMinMaxCurve(module.limit);
		}
		m_writer.Write(module.dampen);
	}

	static void WriteParticleSystemInheritVelocity(ParticleSystem.InheritVelocityModule module) {
		m_writer.Write(module.enabled);
		if(!module.enabled) {
			return;
		}

		m_writer.Write((int) module.mode);
		WriteMinMaxCurve(module.curve);
	}

	static void WriteParticleSystemForceOverLifetime(ParticleSystem.ForceOverLifetimeModule module) {
		m_writer.Write(module.enabled);
		if(!module.enabled) {
			return;
		}

		WriteMinMaxCurve(module.x);
		WriteMinMaxCurve(module.y);
		WriteMinMaxCurve(module.z);
		m_writer.Write((int) module.space);
		m_writer.Write(module.randomized);
	}

	static void WriteParticleSystemColorOverLifetime(ParticleSystem.ColorOverLifetimeModule module) {
		m_writer.Write(module.enabled);
		if(!module.enabled) {
			return;
		}

		WriteMinMaxGradient(module.color);
	}

	static void WriteParticleSystemColorBySpeed(ParticleSystem.ColorBySpeedModule module) {
		m_writer.Write(module.enabled);
		if(!module.enabled) {
			return;
		}

		WriteMinMaxGradient(module.color);
		WriteVector2(module.range);
	}

	static void WriteParticleSystemSizeOverLifetime(ParticleSystem.SizeOverLifetimeModule module) {
		m_writer.Write(module.enabled);
		if(!module.enabled) {
			return;
		}

		m_writer.Write(module.separateAxes);
		if(module.separateAxes) {
			WriteMinMaxCurve(module.x);
			WriteMinMaxCurve(module.y);
			WriteMinMaxCurve(module.z);
		} else {
			WriteMinMaxCurve(module.size);
		}
	}

	static void WriteParticleSystemSizeBySpeed(ParticleSystem.SizeBySpeedModule module) {
		m_writer.Write(module.enabled);
		if(!module.enabled) {
			return;
		}

		m_writer.Write(module.separateAxes);
		if(module.separateAxes) {
			WriteMinMaxCurve(module.x);
			WriteMinMaxCurve(module.y);
			WriteMinMaxCurve(module.z);
		} else {
			WriteMinMaxCurve(module.size);
		}
		WriteVector2(module.range);
	}

	static void WriteParticleSystemRotationOverLifetime(ParticleSystem.RotationOverLifetimeModule module) {
		m_writer.Write(module.enabled);
		if(!module.enabled) {
			return;
		}

		m_writer.Write(module.separateAxes);
		if(module.separateAxes) {
			WriteMinMaxCurve(module.x);
			WriteMinMaxCurve(module.y);
			WriteMinMaxCurve(module.z);
		} else {
			WriteMinMaxCurve(module.z);
		}
	}

	static void WriteParticleSystemRotationBySpeed(ParticleSystem.RotationBySpeedModule module) {
		m_writer.Write(module.enabled);
		if(!module.enabled) {
			return;
		}

		m_writer.Write(module.separateAxes);
		if(module.separateAxes) {
			WriteMinMaxCurve(module.x);
			WriteMinMaxCurve(module.y);
			WriteMinMaxCurve(module.z);
		} else {
			WriteMinMaxCurve(module.z);
		}
		WriteVector2(module.range);
	}

	static void WriteParticleSystemExternalForces(ParticleSystem.ExternalForcesModule module) {
		m_writer.Write(module.enabled);
		if(!module.enabled) {
			return;
		}

		m_writer.Write(module.multiplier);
	}

	static void WriteParticleSystemTextureSheetAnimation(ParticleSystem.TextureSheetAnimationModule module) {
		m_writer.Write(module.enabled);
		if(!module.enabled) {
			return;
		}

		m_writer.Write(module.numTilesX);
		m_writer.Write(module.numTilesY);
		m_writer.Write((int) module.animation);
		if(module.animation == ParticleSystemAnimationType.SingleRow) {
			m_writer.Write(module.useRandomRow);
			m_writer.Write(module.rowIndex);
		}

		WriteMinMaxCurve(module.frameOverTime);
		WriteMinMaxCurve(module.startFrame);
		m_writer.Write(module.cycleCount);
		m_writer.Write(module.flipU);
		m_writer.Write(module.flipV);
		m_writer.Write((int) module.uvChannelMask);
	}

	static void WriteMinMaxCurve(ParticleSystem.MinMaxCurve curve) {
		m_writer.Write((int) curve.mode);
		if(curve.mode == ParticleSystemCurveMode.Constant) {
			m_writer.Write(curve.constant);
		} else if(curve.mode == ParticleSystemCurveMode.Curve) {
			WriteAnimationCurve(curve.curve);
		} else if(curve.mode == ParticleSystemCurveMode.TwoCurves) {
			WriteAnimationCurve(curve.curveMin);
			WriteAnimationCurve(curve.curveMax);
		} else if(curve.mode == ParticleSystemCurveMode.TwoConstants) {
			m_writer.Write(curve.constantMin);
			m_writer.Write(curve.constantMax);
		}
	}

	static void WriteMinMaxGradient(ParticleSystem.MinMaxGradient grad) {
		m_writer.Write((int) grad.mode);
		if(grad.mode == ParticleSystemGradientMode.Color) {
			WriteColor(grad.color);
		} else if(grad.mode == ParticleSystemGradientMode.Gradient) {
			WriteGradient(grad.gradient);
		} else if(grad.mode == ParticleSystemGradientMode.TwoColors) {
			WriteColor(grad.colorMin);
			WriteColor(grad.colorMax);
		} else if(grad.mode == ParticleSystemGradientMode.TwoGradients) {
			WriteGradient(grad.gradientMin);
			WriteGradient(grad.gradientMax);
		} else if(grad.mode == ParticleSystemGradientMode.RandomColor) {
			WriteGradient(grad.gradient);
		}
	}

	static void WriteGradient(Gradient grad) {
		m_writer.Write((int) grad.mode);

		var colorKeys = grad.colorKeys;
		var alphaKeys = grad.alphaKeys;

		m_writer.Write(colorKeys.Length);
		for(int i = 0; i < colorKeys.Length; i++) {
			m_writer.Write(colorKeys[i].time);
			WriteColor(colorKeys[i].color);
		}

		m_writer.Write(alphaKeys.Length);
		for(int i = 0; i < alphaKeys.Length; i++) {
			m_writer.Write(alphaKeys[i].time);
			m_writer.Write(alphaKeys[i].alpha);
		}
	}
}