module Rails
  class Configuration
  
    private
      def default_log_path
        ENV['LOG_FILE_PATH']
      end
   end
end
