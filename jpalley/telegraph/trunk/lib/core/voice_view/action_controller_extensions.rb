module ActionController
  class Base
    def render_voice(&block)
      begin
        yield response.interface
      end
      @performed_render = true
    end

    #generic action that can be used to detect hangups
    def call_hung_up
      if respond_to?(:hung_up)
        hung_up
      end
    end

    #used to update session before action completes
    def update_session(key,val)
      session[key]=val
      session.update
    end
  end
end
