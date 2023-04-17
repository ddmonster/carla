from typing import Optional
import dreyevr  # calls __init__.py which ensures PythonPath contains the necessary files!
import carla


def find_ego_vehicle(world: carla.libcarla.World) -> Optional[carla.libcarla.Vehicle]:
    DReyeVR_vehicles: str = "harplab.dreyevr_vehicle.*"
    ego_vehicles_in_world = list(world.get_actors().filter(DReyeVR_vehicles))
    if len(ego_vehicles_in_world) >= 1:
        print(
            f"Found a DReyeVR EgoVehicle in the world ({ego_vehicles_in_world[0].id})"
        )
        return ego_vehicles_in_world[0]

    DReyeVR_vehicle: Optional[carla.libcarla.Vehicle] = None
    available_ego_vehicles = world.get_blueprint_library().filter(DReyeVR_vehicles)
    if len(available_ego_vehicles) == 1:
        bp = available_ego_vehicles[0]
        print(f'Spawning only available EgoVehicle: "{bp.id}"')
    else:
        print(
            f"Found {len(available_ego_vehicles)} available EgoVehicles. Which one to use?"
        )
        for i, ego in enumerate(available_ego_vehicles):
            print(f"\t[{i}] - {ego.id}")
        print()
        ego_choice = f"Pick EgoVehicle to spawn [0-{len(available_ego_vehicles) - 1}]: "
        i: int = int(input(ego_choice))
        assert 0 <= i < len(available_ego_vehicles)
        bp = available_ego_vehicles[i]
    i: int = 0
    spawn_pts = world.get_map().get_spawn_points()
    while DReyeVR_vehicle is None:
        print(f'Spawning DReyeVR EgoVehicle: "{bp.id}" at {spawn_pts[i]}')
        DReyeVR_vehicle = world.spawn_actor(bp, transform=spawn_pts[i])
        i = (i + 1) % len(spawn_pts)
    return DReyeVR_vehicle


def find_ego_sensor(world: carla.libcarla.World) -> Optional[carla.libcarla.Sensor]:
    sensor = None
    ego_sensors = list(world.get_actors().filter("harplab.dreyevr_sensor.*"))
    if len(ego_sensors) >= 1:
        sensor = ego_sensors[0]  # TODO: support for multiple ego sensors?
    elif find_ego_vehicle(world) is None:
        raise Exception(
            "No EgoVehicle (nor EgoSensor) found in the world! EgoSensor needs EgoVehicle as parent"
        )
    return sensor
