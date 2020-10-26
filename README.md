# Ant War

Author: Jiasheng Zhou \<jiashen2@andrew.cmu.edu\>

Design: It's a two-player RTS game where each player controls an ant kingdom and can build, upgrade and sell structures and summon ants to take over the opponent's kingdom.

Networking: \
The network model contains three parts.\
The first part is the starter code Connection.hpp and Connection.cpp which provides an abstraction of underlying connection between one client and one server.\
The second part is a network serializer and deserializer in Message.hpp and Message.cpp which you can build your protocol between client and server on. It also contains a dispatcher where it can process registered callback according to operation id in the header upon receiving packet. \
The third part is the client and server RPC calls in Player.hpp. The stub function serializes the parameters and impl method actually executes the function. The state update and action client performed is all communicated using RPC calls.

Screen Shot:

![Screen Shot](screenshot.png)

How To Play:

Goals: Defeat your opponent's ant kingdom.
Controls: \
WASD: Move Camera\
Arrow keys: Dialog\

Sources: \
Black ant 3d model: 
Red ant 3d model: 

This game was built with [NEST](NEST.md).

