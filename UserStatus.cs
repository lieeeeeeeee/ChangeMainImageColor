using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace testProject {
    internal class UserStatus {
        public static int mp { get; set; } = 0;
        public static int attack { get; set; } = 0;
        public static bool reflection { get; set; } = false;
        public static bool tackle { get; set; }  = false;
        public static bool sustaining { get; set; } = false;
        public static bool explosion { get; set; } = false;
        public static string weapon { get; set; } = "";
        public static string range { get; set; } = "";
        public static string size { get; set; } = "";
        public static string color { get; set; } = "";

        //10,10,true,true,true,true, "sword","long", 'smorl',"red"
        public object this[string propertyName] {
            get {
                return typeof(UserStatus).GetProperty(propertyName).GetValue(this);
            }
            set {
                try {
                    typeof(UserStatus).GetProperty(propertyName).SetValue(this, value);
                } catch(Exception ex) {
                    Console.WriteLine(ex.ToString());
                }
            }
        }
    }
    
}
