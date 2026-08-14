export default researchConductor = `
Input:
      Simulation: choice (1: Custom, 2: Orbital Kinetic Strike, 3: Operation Midnight Hammer), hammerChoice / numBombs (salvo: 2, 4, 6 bombs), tos_agree.

      Projectile: name, length, diameter, curvature_noseReduce, total_mass, explosive_mass, explosive_energy_j_per_kg, casing_density, yield_strength, area_moment_inertia, elastic_modulus, casing_wall_thickness, hugoniot_c0, hugoniot_s, explosive_critical_energy, specific_heat, melting_point, heat_of_fusion.
      
      Target: name, layers [material_name, thickness, density, compressive_strength, rebar_volume_fraction, rebar_yield_strength, pulverized_depth, hugoniot_c0, hugoniot_s].
      
      Scenario: numScenarios, name_ss / name, dropAltitude_ft / altitude_ft, initial_velocity / velocity, obliquity / obliquity_angle, fpa / flight_path_angle, aoa / angle_of_attack.

      Physics & Environment Constants:
        - PhysicsConstants: gravity, PI, frictionFactor, universalGasConstant, molarMassAir, adiabaticIndexAir, earthRadius, SpeedOfSound.
        - AtmosphereState: temperature_K, pressure_Pa, density_kgm3, speed_of_sound_ms.
        - Aircraft: bomber_totalMass, bomber_wingArea, bomber_liftCurveSlope.

Output:
`;
