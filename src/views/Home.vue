<template>
  <div class="home">
    <v-container>
      <v-row>
        <v-col cols="6">
          <v-row>
            <v-col cols="12">
              <div class="title-box">
                <div class="title">
                  Last Update:
                  <p id="intext">{{ this.last_update }}</p>
                </div>
              </div>
            </v-col>
            <v-col cols="12">
              <div class="title-box">
                <div class="title">
                  Temperature:
                  <p id="intext">{{ this.temperature }}°C</p>
                </div>
              </div>
            </v-col>
            <v-col cols="12">
              <div class="title-box">
                <div class="title">
                  FAN pwm:
                  <p id="intext">{{ this.pwm }}</p>
                </div>
              </div>
            </v-col>
            <v-col cols="12">
              <div class="title-box">
                <div class="bbtn title">
                  <v-btn :disabled="btnDisable" @click="update">Refresh </v-btn>
                </div>
              </div>
            </v-col>
          </v-row>
        </v-col>

        <v-col>
          <v-row>
            <v-col cols="12">
              <div class="automode">
                <v-sheet
                  width="100%"
                  height="100%"
                  class="pa-6"
                  style="background-color: rgba(250, 235, 215, 0)"
                >
                  <v-switch
                    v-model="automode"
                    hide-details
                    inset
                    :disabled="toggling"
                    :label="`${mode}`"
                    @change="swtichmode()"
                    color="white"
                  >
                  </v-switch>
                </v-sheet>
                <v-fade-transition hide-on-leave>
                  <v-card
                    v-if="toggling"
                    append-icon="$close"
                    class="toggle_card"
                    elevation="16"
                    height="200"
                    width="250"
                    color="#fff"
                  >
                    <v-card-text class="sending_direct_method">
                      Sending Direct Method...
                      <v-progress-circular
                        class="progess"
                        :indeterminate="toggling"
                        :value="20"
                        size="48"
                        color="blue"
                        width="8"
                        hide-details="false"
                      ></v-progress-circular>
                    </v-card-text>
                  </v-card>
                </v-fade-transition>
              </div>
            </v-col>
            <v-col cols="12" v-if="!automode">
              <v-form class="manualMode">
                <v-text-field
                  class="fan_input"
                  label="PWM (0~256)"
                  auto-grow
                  outlined
                  rows="1"
                  row-height="15"
                  v-model="setPwm"
                ></v-text-field>
                <div class="manulaMode_btn">
                  <v-btn :disabled="!PwmFormat" @click="changeFanSpeed"
                    >Send
                  </v-btn>
                </div>
              </v-form>
            </v-col>
          </v-row>
        </v-col>
      </v-row>
    </v-container>
  </div>
</template>

<script>
// @ is an alias to /src

export default {
  name: "Home",
  data() {
    return {
      model: true,
      temperature: 0,
      last_update: 0,
      pwm: 0,
      automode: null,
      btnDisable: false,
      toggling: false,
      setPwm: 0,
    };
  },
  computed: {
    mode() {
      return this.automode ? "Auto Mode" : "Manual Mode";
    },
    PwmFormat() {
      return this.pwm_formatcheck();
    },
  },
  watch: {},
  components: {},
  methods: {
    update() {
      this.$axios
        .get(`https://wemosd1dp.azurewebsites.net/`)
        .then((result) => {
          console.log(result);
          this.temperature = result.data.temperature.toFixed(2);
          this.last_update = result.data.last_update;
          this.pwm = result.data.pwm;
          this.automode = result.data.automode;
        })
        .catch((err) => {
          console.log(err);
        });
      // this.btnDisable = !this.btnDisable
    },

    swtichmode() {
      this.toggling = true;
      let mode = this.automode ? 1 : 0;
      console.log("mode:", mode);
      this.$axios
        .post(`https://wemosd1dp.azurewebsites.net/api/changemode`, {
          mode: mode,
        })
        .then((result) => {
          console.log("changemode API result:", result);
          if (result.data.payload.status == "OK") {
            this.automode = !this.automode;
            setTimeout(() => {
              this.toggling = false;
              this.update();
            }, 1500);
          } else {
            setTimeout(() => {
              this.toggling = false;
              this.update();
            }, 2500);
            alert("Failed to send direct method");
          }
        })
        .catch((err) => {
          setTimeout(() => {
            this.toggling = false;
            alert("Failed to send direct method");
          }, 2000);
          console.log(err);
        });
    },
    changeFanSpeed() {
      if (parseInt(this.setPwm) < 0 || parseInt(this.setPwm) > 256) {
        alert("PWM should be 0 ~ 256");
      } else {
        this.toggling = true;

        this.$axios
          .post(`https://wemosd1dp.azurewebsites.net/api/changefanspeed`, {
            fanspeed: parseInt(this.setPwm),
          })
          .then((result) => {
            console.log(result);
            setTimeout(() => {
              this.toggling = false;
              this.update();
            }, 2000);
          })
          .catch((err) => {
            setTimeout(() => {
              this.toggling = false;
              alert("Failed to send direct method");
            }, 1500);
            console.log(err);
          });
      }
    },
    pwm_formatcheck() {
      if (parseInt(this.setPwm) || parseInt(this.setPwm) == 0) {
        console.log(parseInt(this.setPwm));
        if (parseInt(this.setPwm) >= 0 && parseInt(this.setPwm) <= 256) {
          return true;
        }
        return false;
      } else return false;
    },
  },
  created() {
    console.log("=== Home created! === ");
    this.$axios
      .get(`https://wemosd1dp.azurewebsites.net/`)
      .then((result) => {
        this.temperature = result.data.temperature.toFixed(2);
        this.last_update = result.data.last_update;
        this.pwm = result.data.pwm;
        this.automode = result.data.automode;
      })
      .catch((err) => {
        console.log(err);
      });
  },
  beforeUpdate() {
    console.log("Pwm", this.PwmFormat);
    console.log("=== Home beforeUpdate! === ");
  },
  updated() {
    console.log("=== Home updated! === ");
  },
};
</script>
<style lang="scss" scoped>
* {
  // border: 1px solid #f00;
  color: #fff;
}

.home {
  width: 100vw;
  height: 100vh;
  padding-top: 20px;
  // position: relative;
  // background-color: white;
  background-image: linear-gradient(
    0deg,
    rgb(220, 224, 228) 0%,
    rgb(52, 148, 192) 100%
  );
  background-size: cover;
}

.title {
  color: rgb(255, 255, 255);
  width: 200px;
}

#intext {
  font-size: 20px;
}

.bbtn {
  border-radius: 5px;
  height: 30px;
  // margin-left: 40px;
}

.toggle_card {
  position: absolute;
  top: 30%;
  left: 50%;
  transform: translate(-50%, -50%);
  z-index: 99;
  // height: auto;
  // margin-top: 50px;
}

.automode {
  // position: absolute;
  .v-input--selection-controls {
    margin-top: 0;
  }

  // margin-left: 100px;
}

.title-box {
  // padding-left: 15px;
  display: flex;
  justify-content: flex-end;
}

.btn {
  position: relative;
  // right: 50%;
  top: 20%;
  left: 32%;
}

.manulaMode_btn {
  position: relative;
  // top: 12%;
  // bottom: 5%;
  // left: 30%;
  height: 30px;
  // max-height: 30px;
}

.manualMode {
  position: relative;
  // margin-top: 20px;
  // margin-left: 20px;
  // top: 30%;
  // left: 30%;
  width: 65%;
}

.fan_input {
  font-size: 18px;
}

.progess {
  margin-top: 30px;
}

.sending_direct_method {
  position: relative;
  font-size: 19px;
  font-weight: 800;
  text-align: center;
  margin-top: 30px;
}
</style>
