namespace FreeSwitch.EventSocket
{
    /// <summary>
    /// Determines how important the log message is.
    /// </summary>
    public enum LogPrio
    {
        /// <summary>
        /// Not important at all, used to follow the flow of the program
        /// </summary>
        Trace,

        /// <summary>
        /// Messages useful when debugging
        /// </summary>
        Debug,

        /// <summary>
        /// States etc have changed.
        /// </summary>
        Info,

        /// <summary>
        /// Something did not go as we expected, but we can handle it.
        /// </summary>
        Warning,

        /// <summary>
        /// Something went wrong, we can't really handle it, but the program can continue to live
        /// </summary>
        Error,

        /// <summary>
        /// Some went really wrong, and the program should really be shut down.
        /// </summary>
        Fatal
    }

    /// <summary>
    /// Used to retreive logging messages from the event socket framework.
    /// </summary>
    /// <param name="prio">importance of the log message</param>
    /// <param name="message">the message</param>
    public delegate void LogWriterHandler(LogPrio prio, string message);

    /// <summary>
    /// Used as default logwriter
    /// </summary>
    internal class NullWriter
    {
        /// <summary>
        /// Used to log to void.
        /// </summary>
        /// <param name="prio"></param>
        /// <param name="message"></param>
        public static void Write(LogPrio prio, string message)
        {}
    }
}
