/*
 * This file is part of iNav
 *
 * iNav is free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * iNav is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "drivers/persistent.h"
#include <stdbool.h>

bool mscCheckBoot(void)
{
    return (persistentObjectRead(PERSISTENT_OBJECT_RESET_REASON) == RESET_MSC_REQUEST);
}
