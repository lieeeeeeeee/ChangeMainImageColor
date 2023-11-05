
using System.Diagnostics;
using System.Text.RegularExpressions;
using static System.Console;
using System;
using System.Reflection;

namespace testProject {
    internal class SetUserStatus {
        static void SetValue() {
            Console.WriteLine("pls input a userStatus");
            UserStatus userStatus = new UserStatus();
            string? message = Console.ReadLine();
            string[] userStatuses;
            char[] removeChars = new char[] { ' ', '\"', '\'' };
            Type type = typeof(UserStatus);
            var properties = userStatus.GetType().GetProperties();

            Regex integer = new Regex("^[0-9]+$");
            Regex boolean = new Regex("false|true|t|f");

            int membersCount;
            int userStatusCount;

            if (message == null) return;
            foreach (char c in removeChars) message = message.Replace(c.ToString(), "");

            userStatuses = Regex.Split(message, @",");

            membersCount = properties.Length;
            userStatusCount = userStatuses.Length;

            if (membersCount == userStatusCount) return;

            for (int i = 0; i < userStatuses.Length; i++) {
                var propertiesName = properties[i].Name;
                string value = userStatuses[i].ToLower();

                if(integer.IsMatch(value)) { userStatus[propertiesName] = Int32.Parse(value); }
                else if (boolean.IsMatch(value)) { userStatus[propertiesName] = Convert.ToBoolean(value); }
                else { userStatus[propertiesName] = value; }
                
                Console.WriteLine(i + ": " + userStatuses[i]);
                Console.WriteLine(propertiesName + ": " + userStatus[propertiesName]);
            }
            Console.WriteLine(UserStatus.mp);
        }
    }
}
