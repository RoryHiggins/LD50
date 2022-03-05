#include <od/engine/lua/client.hpp>

#include <cstddef>

#include <od/core/debug.h>

#include <od/engine/lua/includes.h>
#include <od/engine/lua/wrappers.h>
#include <od/engine/lua/bindings.h>

bool odLuaClient_init(odLuaClient* client) {
	if (!OD_CHECK(client != nullptr)) {
		return false;
	}

	odLuaClient_destroy(client);

	client->lua = luaL_newstate();

	if (!OD_CHECK(client->lua != nullptr)) {
		return false;
	}

	luaL_openlibs(client->lua);

	if (!OD_CHECK(odLuaBindings_register(client->lua))) {
		return false;
	}

	lua_settop(client->lua, 0);

	return true;
}
void odLuaClient_destroy(odLuaClient* client) {
	if (!OD_CHECK(client != nullptr)) {
		return;
	}

	if (client->lua != nullptr) {
		lua_close(client->lua);
	}

	client->lua = nullptr;
}
void odLuaClient_swap(odLuaClient* client1, odLuaClient* client2) {
	if (!OD_CHECK(client1 != nullptr)
		|| !OD_CHECK(client2 != nullptr)) {
		return;
	}

	lua_State* lua_swap = client1->lua;
	client1->lua = client2->lua;
	client2->lua = lua_swap;
}
bool odLuaClient_check_valid(const odLuaClient* client) {
	if (!OD_CHECK(client != nullptr)) {
		return false;
	}

	return (client->lua != nullptr);
}
lua_State* odLuaClient_get_lua(odLuaClient* client) {
	if (!OD_CHECK(client != nullptr)) {
		return nullptr;
	}

	return client->lua;
}
odLuaClient::odLuaClient()
: lua{nullptr} {
	OD_DISCARD(OD_CHECK(odLuaClient_init(this)));
}
odLuaClient::odLuaClient(odLuaClient&& other)
: odLuaClient{} {
	odLuaClient_swap(this, &other);
}
odLuaClient& odLuaClient::operator=(odLuaClient&& other) {
	odLuaClient_swap(this, &other);
	return *this;
}
odLuaClient::~odLuaClient() {
	odLuaClient_destroy(this);
}
