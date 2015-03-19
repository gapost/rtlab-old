/*
	A C++ binding for comedilib.  Requires the Boost C++ libraries.
	Copyright (C) 2006-2007  Frank Mori Hess <fmhess@users.sourceforge.net>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef QT_COMEDILIB_WRAPPER
#define QT_COMEDILIB_WRAPPER

#include <QSharedPointer>
#include <comedilib.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <stdlib.h>

namespace comedi
{
	class subdevice;

	// wrapper for comedi_to_physical()
	class to_physical
	{
	public:
		to_physical()
		{
			memset(&_polynomial, 0, sizeof(_polynomial));
		}
		to_physical(const comedi_polynomial_t &polynomial):
			_polynomial(polynomial)
		{}
		double operator()(lsampl_t data) const
		{
			return comedi_to_physical(data, &_polynomial);
		}
	private:
		comedi_polynomial_t _polynomial;
	};

	// wrapper for comedi_from_physical()
	class from_physical
	{
	public:
		from_physical()
		{
			memset(&_polynomial, 0, sizeof(_polynomial));
		}
		from_physical(const comedi_polynomial_t &polynomial):
			_polynomial(polynomial)
		{}
		lsampl_t operator()(double physical_value) const
		{
			return comedi_from_physical(physical_value, &_polynomial);
		}
	private:
		comedi_polynomial_t _polynomial;
	};

	class device	{
	public:
		device() {}
		device(const std::string &device_file)
		{
			this->open(device_file);
		}
		std::string board_name() const
		{
			const char *name = comedi_get_board_name(comedi_handle());
			if(name == 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_get_board_name() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_get_board_name");
				throw std::runtime_error(message.str());
			}
			return name;
		}
		int command_test(comedi_cmd *cmd) const
		{
			int retval = comedi_command_test(comedi_handle(), cmd);
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_command_test() returned error.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_command_test");
				throw std::runtime_error(message.str());
			}
			return retval;
		}
		void command(comedi_cmd *cmd) const
		{
			int retval = comedi_command(comedi_handle(), cmd);
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_command() failed, return value=" << retval << " .";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_command");
				throw std::runtime_error(message.str());
			}
		}
		std::string default_calibration_path() const
		{
			char *c_string_file_path = comedi_get_default_calibration_path(comedi_handle());
			if(c_string_file_path == NULL)
			{
                //comedi_perror("comedi_get_default_calibration_path");
				throw std::runtime_error(__PRETTY_FUNCTION__);
			}
			std::string file_path = c_string_file_path;
			free(c_string_file_path);
			return file_path;
		}
		void do_insn(comedi_insn *instruction) const
		{
			int retval = comedi_do_insn(comedi_handle(), instruction);
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_do_insn() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_do_insn");
				throw std::runtime_error(message.str());
			}
		}
		std::string driver_name() const
		{
			const char *name = comedi_get_driver_name(comedi_handle());
			if(name == 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_get_driver_name() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_get_driver_name");
				throw std::runtime_error(message.str());
			}
			return name;
		}
		inline subdevice find_subdevice_by_type(int type, const subdevice *start_subdevice = 0) const;
		int fileno() const
		{
			int fd = comedi_fileno(comedi_handle());
			if(fd < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_fileno() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_fileno");
				throw std::runtime_error(message.str());
			}
			return fd;
		}
		unsigned n_subdevices() const
		{
			int retval = comedi_get_n_subdevices(comedi_handle());
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_get_n_subdevices() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_get_n_subdevices");
				throw std::runtime_error(message.str());
			}
			return retval;
		}
		void open(const std::string &device_file)
		{
			comedi_t *dev = comedi_open(device_file.c_str());
			if(dev == 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_open() failed, with device file name: \"" << device_file << "\".";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_open");
				throw std::runtime_error(message.str().c_str());
			}

            _comedi_handle = QSharedPointer<comedi_t>(dev, &comedi_close);
		}
	private:
		friend class subdevice;

		comedi_t* comedi_handle() const
		{
            if(_comedi_handle.isNull())
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": device not open.";
                //std::cerr << message.str() << std::endl;
				throw std::invalid_argument(message.str());
			}
            return _comedi_handle.data();
		}

        QSharedPointer<comedi_t> _comedi_handle;
	};

	class calibration
	{
	public:
		calibration() {}
		calibration(const device &dev)
		{
			init(dev.default_calibration_path());
		}
		calibration(const std::string &file_path)
		{
			init(file_path);
		}
		const comedi_calibration_t* c_calibration() const
		{
            return _c_calibration.data();
		}
	private:
		void init(const std::string &file_path)
		{
			comedi_calibration_t *cal = comedi_parse_calibration_file(file_path.c_str());
			if(cal == NULL)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_parse_calibration_file() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_parse_calibration_file");
				throw std::runtime_error(message.str());
			}
            _c_calibration = QSharedPointer<comedi_calibration_t>(cal, &comedi_cleanup_calibration);
		}
        QSharedPointer<comedi_calibration_t> _c_calibration;
	};

	class subdevice
	{
	public:
		subdevice(): _index(-1)
		{}
		subdevice(const device &dev, unsigned subdevice_index):
			_device(dev), _index(subdevice_index)
		{
			if(_index >= _device.n_subdevices())
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": invalid subdevice index.";
                //std::cerr << message.str() << std::endl;
				throw std::invalid_argument(message.str());
			}
		}
		void apply_hard_calibration(unsigned channel, unsigned range, unsigned aref, const calibration &cal)
		{
			if(cal.c_calibration() == 0) throw std::invalid_argument(__PRETTY_FUNCTION__);
			int retval = comedi_apply_parsed_calibration(comedi_handle(), index(),
				channel, range, aref, cal.c_calibration());
			if(retval < 0)
			{
                //comedi_perror("comedi_apply_parsed_calibration");
				throw std::runtime_error(__PRETTY_FUNCTION__);
			}
		}
		void apply_hard_calibration(unsigned channel, unsigned range, unsigned aref)
		{
			apply_hard_calibration(channel, range, aref, calibration(dev()));
		}
		unsigned buffer_size() const
		{
			int retval = comedi_get_buffer_size(comedi_handle(), index());
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_get_buffer_size() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_get_buffer_size");
				throw std::runtime_error(message.str());
			}
			return retval;
		}
		void cancel()
		{
			int retval = comedi_cancel(comedi_handle(), index());
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_cancel() failed, return value=" << retval << " .";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_cancel");
				throw std::runtime_error(message.str());
			}
		}
		lsampl_t data_read(unsigned channel, unsigned range, unsigned aref) const
		{
			lsampl_t value;
			int retval = comedi_data_read(comedi_handle(), index(), channel, range, aref, &value);
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_data_read() failed, return value=" << retval << " .";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_data_read");
				throw std::runtime_error(message.str());
			}
			return value;
		}
		std::vector<lsampl_t> data_read_n(unsigned channel, unsigned range, unsigned aref, unsigned num_samples) const
		{
			std::vector<lsampl_t> values(num_samples);
			int retval = comedi_data_read_n(comedi_handle(), index(), channel, range, aref, &values.at(0), values.size());
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_data_read_n() failed, return value=" << retval << " .";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_data_read_n");
				throw std::runtime_error(message.str());
			}
			return values;
		}
		void data_read_hint(unsigned channel, unsigned range, unsigned aref) const
		{
			int ret = comedi_data_read_hint(comedi_handle(), index(), channel, range, aref);
			if(ret < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_data_read_hint() failed, return value = " << ret << " .";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_data_read_hint");
				throw std::runtime_error(message.str());
			}
		}
		void data_write(unsigned channel, unsigned range, unsigned aref, lsampl_t data) const
		{
			int retval = comedi_data_write(comedi_handle(), index(), channel, range, aref, data);
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_data_write() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_data_write");
				throw std::runtime_error(message.str());
			}
		}
		device dev() const {return _device;}
		void dio_bitfield2(unsigned write_mask, unsigned *bits, unsigned base_channel)
		{
			int retval = comedi_dio_bitfield2(comedi_handle(), index(),
				write_mask, bits, base_channel);
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_dio_bitfield2() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_dio_bitfield2");
				throw std::runtime_error(message.str());
			}
		}
        unsigned dio_read(unsigned ch)
        {
            unsigned bit;
            int retval = comedi_dio_read(comedi_handle(), index(),
                ch, &bit);
            if(retval < 0)
            {
                std::ostringstream message;
                message << __PRETTY_FUNCTION__ << ": comedi_dio_read() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_dio_bitfield2");
                throw std::runtime_error(message.str());
            }
            return bit;
        }
        void dio_write(unsigned ch, unsigned bit)
        {
            int retval = comedi_dio_write(comedi_handle(), index(),
                ch, bit);
            if(retval < 0)
            {
                std::ostringstream message;
                message << __PRETTY_FUNCTION__ << ": comedi_dio_write() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_dio_bitfield2");
                throw std::runtime_error(message.str());
            }
        }
        void dio_config(unsigned channel, enum comedi_io_direction direction)
		{
			int retval = comedi_dio_config(comedi_handle(), index(), channel, direction);
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_dio_config() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_dio_config");
				throw std::runtime_error(message.str());
			}
		}
		unsigned find_range(unsigned channel, unsigned unit, double min, double max)
		{
			int retval = comedi_find_range(comedi_handle(), index(), channel,
				unit, min, max);
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_find_range() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_find_range");
				throw std::runtime_error(message.str());
			}
			return retval;
		}
		unsigned flags() const
		{
			int retval = comedi_get_subdevice_flags(comedi_handle(), index());
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_get_subdevice_flags() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_get_subdevice_flags");
				throw std::runtime_error(message.str());
			}
			return retval;
		}
		unsigned get_buffer_contents() const
		{
			int retval = comedi_get_buffer_contents(comedi_handle(), index());
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_get_buffer_contents() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_get_buffer_contents");
				throw std::runtime_error(message.str());
			}
			return retval;
		}
		void get_clock_source(unsigned channel, unsigned *clock, unsigned *period_ns) const
		{
			int retval = comedi_get_clock_source(comedi_handle(), index(), channel, clock, period_ns);
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_get_clock_source() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_get_clock_source");
				throw std::runtime_error(message.str());
			}
		}
		void get_gate_source(unsigned channel, unsigned gate_index, unsigned *gate_source)
		{
			int retval = comedi_get_gate_source(comedi_handle(), index(), channel, gate_index, gate_source);
			if (retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_get_gate_source() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_get_gate_source");
				throw std::runtime_error(message.str());
			}
		}
		unsigned get_routing(unsigned channel) const
		{
			unsigned routing = 0;
			int retval = comedi_get_routing(comedi_handle(), index(), channel, &routing);
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_get_routing() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_get_routing");
				throw std::runtime_error(message.str());
			}
			return routing;
		}
		comedi_polynomial_t hardcal_converter(unsigned channel, unsigned range,
			enum comedi_conversion_direction direction) const
		{
			comedi_polynomial_t result;
			int retval = comedi_get_hardcal_converter(comedi_handle(), index(),
				channel, range, direction, &result);
			if(retval < 0)
			{
                //comedi_perror("comedi_get_hardcal_converter");
				throw std::runtime_error(__PRETTY_FUNCTION__);
			}
			return result;
		}
		unsigned hardware_buffer_size(enum comedi_io_direction direction) const
		{
			int retval = comedi_get_hardware_buffer_size(comedi_handle(), index(), direction);
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_get_hardware_buffer_size() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_get_hardware_buffer_size");
				throw std::runtime_error(message.str());
			}
			return retval;
		}
        unsigned index() const {return _index;}
		void internal_trigger(unsigned trig_num) const
		{
			int retval = comedi_internal_trigger(comedi_handle(), index(), trig_num);
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_internal_trigger() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_internal_trigger");
				throw std::runtime_error(message.str());
			}
		}
		unsigned max_buffer_size() const
		{
			int retval = comedi_get_max_buffer_size(comedi_handle(), index());
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_get_max_buffer_size() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_get_max_buffer_size");
				throw std::runtime_error(message.str());
			}
			return retval;
		}
		unsigned n_channels() const
		{
			int retval = comedi_get_n_channels(comedi_handle(), index());
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_get_n_channels() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_get_n_channels");
				throw std::runtime_error(message.str());
			}
			return retval;
		}
		unsigned n_ranges(unsigned channel = 0) const
		{
			int retval = comedi_get_n_ranges(comedi_handle(), index(), channel);
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_get_n_ranges() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_get_n_ranges");
				throw std::runtime_error(message.str());
			}
			return retval;
		}
		lsampl_t max_data(unsigned channel = 0) const
		{
			lsampl_t value = comedi_get_maxdata(comedi_handle(), index(), channel);
			if(value == 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_get_maxdata() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_get_maxdata");
				throw std::runtime_error(message.str());
			}
			return value;
		}
		const comedi_range* range(unsigned channel, unsigned range_index) const
		{
			comedi_range *cRange = comedi_get_range(comedi_handle(), index(), channel, range_index);
			if(cRange == 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_get_range() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_get_range");
				throw std::runtime_error(message.str());
			}
			return cRange;
		}
		void reset() const
		{
			int retval = comedi_reset(comedi_handle(), index());
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_reset() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_reset");
				throw std::runtime_error(message.str());
			}
		}
        void arm(unsigned source) const
        {
            int retval = comedi_arm(comedi_handle(), index(), source);
            if(retval < 0)
            {
                std::ostringstream message;
                message << __PRETTY_FUNCTION__ << ": comedi_arm() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_arm");
                throw std::runtime_error(message.str());
            }
        }
		void set_buffer_size(unsigned num_bytes) const
		{
			int retval = comedi_set_buffer_size(comedi_handle(), index(), num_bytes);
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_set_buffer_size() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_set_buffer_size");
				throw std::runtime_error(message.str());
			}
		}
		void set_clock_source(unsigned channel, unsigned clock, unsigned period_ns)
		{
			int retval = comedi_set_clock_source(comedi_handle(), index(), channel, clock, period_ns);
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_set_clock_source() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_set_clock_source");
				throw std::runtime_error(message.str());
			}
		}
		void set_counter_mode(unsigned channel, unsigned mode_bits)
		{
			int retval = comedi_set_counter_mode(comedi_handle(), index(), channel, mode_bits);
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_set_counter_mode() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_set_counter_mode");
				throw std::runtime_error(message.str());
			}
		}
		void set_gate_source(unsigned channel, unsigned gate_index, unsigned gate_source)
		{
			int retval = comedi_set_gate_source(comedi_handle(), index(), channel, gate_index, gate_source);
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_set_gate_source() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_set_gate_source");
				throw std::runtime_error(message.str());
			}
		}
		void set_max_buffer_size(unsigned num_bytes) const
		{
			int retval = comedi_set_max_buffer_size(comedi_handle(), index(), num_bytes);
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_set_max_buffer_size() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_set_max_buffer_size");
				throw std::runtime_error(message.str());
			}
		}
		void set_routing(unsigned channel, unsigned routing)
		{
			int retval = comedi_set_routing(comedi_handle(), index(), channel, routing);
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_set_routing() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_set_routing");
				throw std::runtime_error(message.str());
			}
		}
        void set_filter(unsigned channel, unsigned filter)
        {
            int retval = comedi_set_filter(comedi_handle(), index(), channel, filter);
            if(retval < 0)
            {
                std::ostringstream message;
                message << __PRETTY_FUNCTION__ << ": comedi_set_filter() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_set_routing");
                throw std::runtime_error(message.str());
            }
        }
		comedi_polynomial_t softcal_converter(unsigned channel, unsigned range,
			enum comedi_conversion_direction direction, const calibration &cal) const
		{
			if(cal.c_calibration() == 0) throw std::invalid_argument(__PRETTY_FUNCTION__);
			comedi_polynomial_t result;
			int retval = comedi_get_softcal_converter(index(),
				channel, range, direction, cal.c_calibration(), &result);
			if(retval < 0)
			{
                //comedi_perror("comedi_get_softcal_converter");
				throw std::runtime_error(__PRETTY_FUNCTION__);
			}
			return result;
		}
		comedi_subdevice_type subdevice_type() const
		{
			int retval = comedi_get_subdevice_type(comedi_handle(), index());
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_get_subdevice_type() failed.";
                //std::cerr << message.str() << std::endl;
                //comedi_perror("comedi_get_subdevice_type");
				throw std::runtime_error(message.str());
			}
			return comedi_subdevice_type(retval);
		}
        static const char* type_name(comedi_subdevice_type t)
        {
            switch(t)
            {
            case COMEDI_SUBD_UNUSED : return "subdevice is unused by driver";
            case COMEDI_SUBD_AI : return "analog input";
            case COMEDI_SUBD_AO : return "analog output";
            case COMEDI_SUBD_DI : return "digital input";
            case COMEDI_SUBD_DO : return "digital output";
            case COMEDI_SUBD_DIO : return "digital input/output";
            case COMEDI_SUBD_COUNTER : return "counter";
            case COMEDI_SUBD_TIMER : return "timer";
            case COMEDI_SUBD_MEMORY : return "memory, EEPROM, DPRAM";
            case COMEDI_SUBD_CALIB : return "calibration DACs and pots";
            case COMEDI_SUBD_PROC : return "processor, DSP";
            case COMEDI_SUBD_SERIAL : return "serial IO";
            case COMEDI_SUBD_PWM : return "pulse width modulation";
            };
            return "Unknown Type";
        };
	private:
		comedi_t* comedi_handle() const {return dev().comedi_handle();}

		device _device;
		unsigned _index;
	};

	subdevice device::find_subdevice_by_type(int type, const subdevice *start_subdevice) const
	{
		unsigned start_index;
		if(start_subdevice) start_index = start_subdevice->index() + 1;
		else start_index = 0;
		int subdev = comedi_find_subdevice_by_type(comedi_handle(), type, start_index);
		if(subdev < 0)
		{
			std::ostringstream message;
			message << __PRETTY_FUNCTION__ << ": failed to find subdevice of type " << type << " .";
            //std::cerr << message.str() << std::endl;
            //comedi_perror("comedi_find_subdevice_by_type");
			throw std::runtime_error(message.str());
		}
		return subdevice(*this, subdev);
	}
};

#endif	// _COMEDILIB_WRAPPER_HPP
