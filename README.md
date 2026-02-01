# Delivery-Detonation-Source
C++ Source Code for a multiplayer physics platformer in Unreal Engine 5. Features a custom Online Session Subsystem using Epic Online Services (EOS), server-authoritative physics replication, and lobby persistence. (Source only).


# Delivery Detonation (Source Code)

**Role:** Technical Lead & Systems Architect
**Engine:** Unreal Engine 5.6
**Language:** C++

## Overview
This repository contains the core C++ source code for *Delivery Detonation*, a distributed multiplayer physics platformer. The project focuses on client-server replication for physics-based gameplay.

**Note:** This repository contains **source code only**. Game assets (Content/Binaries) are excluded to maintain repository efficiency.

## Key Technical Features
* **Online Session Subsystem:** Custom implementation of the `IOnlineSubsystem` interface to manage session lifecycles, player authentication via EOS, and secure lobby persistence.
* **Authoritative Replication:** Server-side validation logic for physics object interaction to prevent client-side desynchronization.
* **Chaos Physics Integration:** Extended `UCharacterMovementComponent` to handle complex physical state transitions (carrying, throwing, balancing) over the network.
* **Player Mechanics Replication:** Server-side validation logic for all player systems and mechanics.
