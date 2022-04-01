local Debugging = require("engine/core/debugging")
local Testing = require("engine/core/testing")
local Schema = require("engine/core/schema")
local Container = require("engine/core/container")
local Model = require("engine/core/model")
local json = require("engine/lib/json/json")
local Sim = require("engine/engine/sim")
local Client = require("engine/engine/client")
local World = require("engine/engine/world")
local Game = require("engine/engine/game")
local Entity = require("engine/engine/entity")
local Image = require("engine/engine/image")

local debug_checks_enabled = Debugging.debug_checks_enabled
local expensive_debug_checks_enabled = Debugging.expensive_debug_checks_enabled

local Template = {}

Template.Template = {}
Template.Template.Schema = Schema.Object{
	state = Entity.Entity.Schema,
	match_images = Schema.Array(Schema.LabelString),
	match_animations = Schema.Array(Schema.LabelString),
}

Template.WorldSys = World.Sys.new_metatable("template")

