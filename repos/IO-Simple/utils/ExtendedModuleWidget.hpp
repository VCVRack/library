#if!defined EXTENDEDMODULEWIDGET_HPP
#define EXTENDEDMODULEWIDGET_HPP
#include <app.hpp>

/*!
  	\class ExtendedModuleWidget
	\brief Ease the widget setup.
 */
class ExtendedModuleWidget : public rack::ModuleWidget
{
public:
	/*!
	  	\brief Create an input widget and add it to the module.
		\param pos Position of the graphic input slot
		\param inputId Input identifier
		\return Returns the new input port. Unlike rack::createInput, this method returns an
		instance of TInput instead of an instance of rack::Input, and the input port is
		direcly added to this using rack::ModuleWidget::addInput.
		\tparam TInput Type of input port.
		\code
		{
			rack::PJ301MPort* const input = this->createInput<rack::PJ301MPort>({30, 30}, YourModule::YOUR_INPUT_INDEX);

			// use input here...
		}
		\endcode
	 */
	template <class TInput>
	TInput* createInput(rack::Vec const& pos, int const inputId)
	{
		assert( this->module != nullptr );

		TInput* const input = new TInput;

		input->box.pos = pos;
		input->module = this->module;
		input->type = rack::Port::INPUT;
		input->portId = inputId;
		rack::ModuleWidget::addInput(input);
		return input;
	}

	/*!
	  	\brief Create an output widget and add it to the module.
		\param pos Position of the graphic output slot
		\param outputId Output identifier
		\return Returns the new output port. Unlike rack::createOutput, this method returns an
		instance of TOutput instead of an instance of rack::Output, and the output port is
		direcly added to this using rack::ModuleWidget::addOutput.
		\tparam TOutput Type of output port.
		\code
		{
			rack::PJ301MPort* const output = this->createOutput<rack::PJ301MPort>({30, 30}, YourModule::YOUR_INPUT_INDEX);

			// use output here...
		}
		\endcode
	 */
	template <class TOutput>
	TOutput* createOutput(rack::Vec const& pos, int const outputId)
	{
		assert( this->module != nullptr );

		TOutput* const output = new TOutput;

		output->box.pos = pos;
		output->module = module;
		output->type = rack::Port::OUTPUT;
		output->portId = outputId;
		rack::ModuleWidget::addOutput(output);
		return output;
	}

	/*!
	  	\brief Create a param widget and add it to the module.
		\param pos Position of the graphic param slot
		\param paramId Param identifier
		\param minValue The minimum value accepted
		\param maxValue The maximum value accepted
		\param defaultValue The default value
		\return Returns the new param port. Unlike rack::createParam, this method returns an
		instance of TParam instead of an instance of rack::Param, and the param port is
		direcly added to this using rack::ModuleWidget::addParam.
		\tparam TParam Type of param port.
		\code
		{
			rack::PJ301MPort* const param = this->createParam<rack::PJ301MPort>({30, 30}, YourModule::YOUR_INPUT_INDEX);

			// use param here...
		}
		\endcode
	 */
	template <class TParam>
	TParam* createParam(rack::Vec const& pos, int const paramId, float const minValue, float const maxValue, float const defaultValue)
	{
		assert( this->module != nullptr );

		TParam* const param = new TParam;

		param->box.pos = pos;
		param->module = this->module;
		param->paramId = paramId;
		param->setLimits(minValue, maxValue);
		param->setDefaultValue(defaultValue);
		rack::ModuleWidget::addParam(param);
		return param;
	}

	template <class TLight>
	TLight* createLight(rack::Vec const& pos, int const lightId)
	{
		assert( this->module != nullptr );

		TLight* const light = new TLight;

		light->box.pos = pos;
		light->module = this->module;
		light->firstLightId = lightId;
		rack::ModuleWidget::addChild(light);
		return light;
	}
};

#endif
