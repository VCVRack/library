/******************************************************************************
 * Copyright 2017-2018 Valerio Orlandini / Sonus Modular <SonusModular@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/


#include "sonusmodular.hpp"


Plugin *plugin;

void init(rack::Plugin *p)
{
    plugin = p;
    p->slug = "SonusModular";
#ifdef VERSION
    p->version = TOSTRING(VERSION);
#endif
    p->website = "https://sonusmodular.sonusdept.com";
    p->manual = "https://gitlab.com/sonusdept/sonusmodular#sonus-modular";

    p->addModel(modelBitter);
    p->addModel(modelBymidside);
    p->addModel(modelCampione);
    p->addModel(modelChainsaw);
    p->addModel(modelCtrl);
    p->addModel(modelDeathcrush);
    p->addModel(modelHarmony);
    p->addModel(modelLuppolo);
    p->addModel(modelLuppolo3);
    p->addModel(modelMicromacro);
    p->addModel(modelMultimulti);
    p->addModel(modelOktagon);
    p->addModel(modelOsculum);
    p->addModel(modelParamath);
    p->addModel(modelPiconoise);
    p->addModel(modelPusher);
    p->addModel(modelRingo);
    p->addModel(modelScramblase);
    p->addModel(modelTwoff);
    p->addModel(modelYabp);
    /*

    
    
    
    
    
    
    
    
    
    
    */
}
