/**
 *
 * creat by sky in 20171018 
 * discription : database declaration.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __DATABASE_CONFIG_H_
#define __DATABASE_CONFIG_H_

// category of product 
#define GLEWLWYD_TABLE_CATEGORY                 "g_category"
#define GLEWLWYD_TABLE_FACTORY                  "g_factory"
#define GLEWLWYD_TABLE_CONTROLLIST              "g_controllist"
#define GLEWLWYD_TABLE_FUNCTION                 "g_function"
#define GLEWLWYD_TABLE_CATEGORY_FUNCTION        "g_category_function"
#define GLEWLWYD_TABLE_DEVICE_STATE             "g_device_state"

// castegory element 
#ifdef GLEWLWYD_TABLE_CATEGORY
#define ELEMENT_CATEGORY_NAME         "gctg_name"
#define JASON_CATEGORY_KEY             "category"
#endif //GLEWLWYD_TABLE_CATEGORY

#ifdef GLEWLWYD_TABLE_FACTORY
#define ELEMENT_FACTORY_NAME      "gfc_name"
#define JASON_FACTORY_KEY         "factory"
#endif
#endif // end fo databease config h
