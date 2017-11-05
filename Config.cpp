/*
obs-websocket
Copyright (C) 2016-2017	Stéphane Lepin <stephane.lepin@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <obs-frontend-api.h>
#include <util/config-file.h>
#include <string>

#define SECTION_NAME "StreamCommander"
#define PARAM_TOKEN "token"

#include "Config.h"

Config* Config::_instance = new Config();

Config::Config()
{
    // OBS Config defaults
    config_t* obs_config = obs_frontend_get_global_config();

    if (obs_config) {
        config_set_default_string(obs_config,
            SECTION_NAME, PARAM_TOKEN, token);
    }
}

Config::~Config() {
}

void Config::Load() {
    config_t* obs_config = obs_frontend_get_global_config();

    token = config_get_string(obs_config, SECTION_NAME, PARAM_TOKEN);
}

void Config::Save() {
    config_t* obs_config = obs_frontend_get_global_config();

    config_set_string(obs_config, SECTION_NAME, PARAM_TOKEN, token);

    config_save(obs_config);
}

Config* Config::Current() {
    return _instance;
}
