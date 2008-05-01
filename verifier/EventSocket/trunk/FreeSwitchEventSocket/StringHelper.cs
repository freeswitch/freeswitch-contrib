namespace FreeSwitch.EventSocket
{
    public static class StringHelper
    {
        /// <summary>
        /// This method converts a enum value to the correct format
        /// that FreeSwitch uses (WORD1_WORD2) instead of the CamelCase that
        /// we use in .net.
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public static string UpperCaseToCamelCase(string name)
        {
            bool isFirst = true;
            string result = string.Empty;
            foreach (char ch in name)
            {
                if (isFirst)
                {
                    result += char.ToUpper(ch);
                    isFirst = false;
                }
                else
                {
                    if (ch == '_')
                        isFirst = true;
                    else
                        result += char.ToLower(ch);
                }
            }
            return result;
        }

        public static string CamelCaseToUpperCase(string name)
        {
            string result = string.Empty;
            bool isFirst = true;
            foreach (char ch in name)
            {
                if (char.IsUpper(ch))
                {
                    if (!isFirst)
                        result += '_';
                    else
                        isFirst = false;
                    result += char.ToUpper(ch);
                }
                else
                    result += char.ToUpper(ch);
            }

            return result;
        }
    }
}