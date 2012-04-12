#ifndef TH_GENERIC_FILE
#define TH_GENERIC_FILE "generic/SqrtOmp.c"
#else

static int nnOmp_(Sqrt_updateOutputOmp)(lua_State *L)
{
  THTensor *input = luaT_checkudata(L, 2, torch_(Tensor_id));
  real bias = luaT_getfieldchecknumber(L,1,"eps");
  setompnthread(L,1,"nThread");
  THTensor *output = luaT_getfieldcheckudata(L, 1, "output", torch_(Tensor_id));

  THTensor_(resizeAs)(output, input);

  if (input->nDimension == 1 || !THTensor_(isContiguous)(input) || !THTensor_(isContiguous)(output))
  {
    TH_TENSOR_APPLY2(real, output, real, input,		\
		     *output_data = sqrt(*input_data + bias););
  }
  else
  {
    real* output_data = THTensor_(data)(output);
    real* input_data  = THTensor_(data)(input);
    long i;
#pragma omp parallel for private(i)
    for(i = 0; i < THTensor_(nElement)(input); i++)
      output_data[i] = sqrt(input_data[i] + bias);
  }
  return 1;
}

static int nnOmp_(Sqrt_updateGradInputOmp)(lua_State *L)
{
  THTensor *input = luaT_checkudata(L, 2, torch_(Tensor_id));
  setompnthread(L,1,"nThread");
  THTensor *gradOutput = luaT_checkudata(L, 3, torch_(Tensor_id));
  THTensor *output = luaT_getfieldcheckudata(L, 1, "output", torch_(Tensor_id));
  THTensor *gradInput = luaT_getfieldcheckudata(L, 1, "gradInput", torch_(Tensor_id));

  THTensor_(resizeAs)(gradInput, input);

  if (output->nDimension == 1 || 
      !THTensor_(isContiguous)(output) || 
      !THTensor_(isContiguous)(gradOutput) ||
      !THTensor_(isContiguous)(gradInput))
  {
    TH_TENSOR_APPLY3(real, gradInput, real, gradOutput, real, output,	\
		     *gradInput_data  = 0.5 * (*gradOutput_data / *output_data););
  }
  else
  {
    real* gradOutput_data = THTensor_(data)(gradOutput);
    real* gradInput_data  = THTensor_(data)(gradInput);
    real* output_data     = THTensor_(data)(output);
    long i;
#pragma omp parallel for private(i)
    for(i = 0; i < THTensor_(nElement)(output); i++)
      gradInput_data[i] = 0.5 * (gradOutput_data[i] / output_data[i]);
  }
  return 1;
}

static const struct luaL_Reg nnOmp_(Sqrt__) [] = {
  {"Sqrt_updateOutputOmp", nnOmp_(Sqrt_updateOutputOmp)},
  {"Sqrt_updateGradInputOmp", nnOmp_(Sqrt_updateGradInputOmp)},
  {NULL, NULL}
};

static void nnOmp_(Sqrt_init)(lua_State *L)
{
  luaT_pushmetaclass(L, torch_(Tensor_id));
  lua_getfield(L,-1,"nn");
  luaL_register(L, NULL, nnOmp_(Sqrt__));
  lua_pop(L,1);
}

#endif