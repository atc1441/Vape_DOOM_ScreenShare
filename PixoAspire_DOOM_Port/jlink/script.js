int InitEMU(void) {
  JLINK_ExecCommand("SuppressInfoUpdateFW");
  JLINK_ExecCommand("DisableAutoUpdateFW");
  return 0;
}
int ConfigTargetSettings(void) { return 0; }
