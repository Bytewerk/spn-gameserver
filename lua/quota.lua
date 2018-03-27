local instruction_counter = 0

function set_quota(max_num_events, max_time_secs)
	local events_per_check = 1000
	local start_time = os.clock()

	function check()
		instruction_counter = instruction_counter + events_per_check
		if instruction_counter > max_num_events then
			debug.sethook()
			error("instruction quota exceeded")
		end

		if os.clock()-start_time > max_time_secs then
			debug.sethook()
			error("time quota exceeded")
		end
	end

	instruction_counter = 0
	debug.sethook(check, "", events_per_check)
end
